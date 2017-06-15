#include "util.h"
#include "helper.h"
#include "MutexFactory.h"
#include "SecureMemoryRegistry.h"
#include "CryptoFactory.h"
#include "Configuration.h"
#include "SimpleConfigLoader.h"
#include "ObjectStoreToken.h"
#include "Directory.h"
#include "OSPathSep.h"

#if defined(WITH_OPENSSL)
#include "OSSLCryptoFactory.h"
#else
#include "BotanCryptoFactory.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#else
#include <direct.h>
#include <io.h>
#endif

#ifdef HAVE_CXX11

std::unique_ptr<MutexFactory> MutexFactory::instance(nullptr);
std::unique_ptr<SecureMemoryRegistry> SecureMemoryRegistry::instance(nullptr);
#if defined(WITH_OPENSSL)
std::unique_ptr<OSSLCryptoFactory> OSSLCryptoFactory::instance(nullptr);
#else
std::unique_ptr<BotanCryptoFactory> BotanCryptoFactory::instance(nullptr);
#endif

#else

std::auto_ptr<MutexFactory> MutexFactory::instance(NULL);
std::auto_ptr<SecureMemoryRegistry> SecureMemoryRegistry::instance(NULL);
#if defined(WITH_OPENSSL)
std::auto_ptr<OSSLCryptoFactory> OSSLCryptoFactory::instance(NULL);
#else
std::auto_ptr<BotanCryptoFactory> BotanCryptoFactory::instance(NULL);
#endif

#endif

bool initSoftHSM()
{
	// Not using threading
	MutexFactory::i()->disable();

	// Initiate SecureMemoryRegistry
	if (SecureMemoryRegistry::i() == NULL)
	{
		fprintf(stderr, "ERROR: Could not initiate SecureMemoryRegistry.\n");
		return false;
	}

	// Build the CryptoFactory
	if (CryptoFactory::i() == NULL)
	{
		fprintf(stderr, "ERROR: Could not initiate CryptoFactory.\n");
		return false;
	}

#ifdef WITH_FIPS
	// Check the FIPS status
	if (!CryptoFactory::i()->getFipsSelfTestStatus())
	{
		fprintf(stderr, "ERROR: FIPS self test failed.\n");
		return false;
	}
#endif

	// Load the configuration
	if (!Configuration::i()->reload(SimpleConfigLoader::i()))
	{
		fprintf(stderr, "ERROR: Could not load the SoftHSM configuration.\n");
		return false;
	}

	// Configure the log level
	if (!setLogLevel(Configuration::i()->getString("log.level", DEFAULT_LOG_LEVEL)))
	{
		fprintf(stderr, "ERROR: Could not configure the log level.\n");
		return false;
	}

	// Configure object store storage backend used by all tokens.
	if (!ObjectStoreToken::selectBackend(Configuration::i()->getString("objectstore.backend", DEFAULT_OBJECTSTORE_BACKEND)))
	{
		fprintf(stderr, "ERROR: Could not select token backend.\n");
		return false;
	}

	return true;
}

void finalizeSoftHSM()
{
	CryptoFactory::reset();
	SecureMemoryRegistry::reset();
}

// Find the token directory
bool findTokenDirectory(std::string basedir, std::string& tokendir, char* serial, char* label)
{
	if (serial == NULL && label == NULL)
	{
		return false;
	}

	// Load the variables
	CK_UTF8CHAR paddedSerial[16];
	CK_UTF8CHAR paddedLabel[32];
	if (serial != NULL)
	{
		size_t inSize = strlen(serial);
		size_t outSize = sizeof(paddedSerial);
		if (inSize > outSize)
		{
			fprintf(stderr, "ERROR: --serial is too long.\n");
			return false;
		}
		memset(paddedSerial, ' ', outSize);
		memcpy(paddedSerial, serial, inSize);
	}
	if (label != NULL)
	{
		size_t inSize = strlen(label);
		size_t outSize = sizeof(paddedLabel);
		if (inSize > outSize)
		{
			fprintf(stderr, "ERROR: --token is too long.\n");
			return false;
		}
		memset(paddedLabel, ' ', outSize);
		memcpy(paddedLabel, label, inSize);
	}

	// Find all tokens in the specified path
	Directory storeDir(basedir);

	if (!storeDir.isValid())
	{
		fprintf(stderr, "Failed to enumerate object store in %s", basedir.c_str());

		return false;
	}

	// Assume that all subdirectories are tokens
	std::vector<std::string> dirs = storeDir.getSubDirs();

	ByteString tokenLabel;
	ByteString tokenSerial;
	CK_UTF8CHAR paddedTokenSerial[16];
	CK_UTF8CHAR paddedTokenLabel[32];
	size_t counter = 0;
	for (std::vector<std::string>::iterator i = dirs.begin(); i != dirs.end(); i++)
	{
		memset(paddedTokenSerial, ' ', sizeof(paddedTokenSerial));
		memset(paddedTokenLabel, ' ', sizeof(paddedTokenLabel));

		// Create a token instance
		ObjectStoreToken* token = ObjectStoreToken::accessToken(basedir, *i);

		if (!token->isValid())
		{
			delete token;
			continue;
		}

		if (token->getTokenLabel(tokenLabel) && tokenLabel.size() <= sizeof(paddedTokenLabel))
		{
			strncpy((char*)paddedTokenLabel, (char*)tokenLabel.byte_str(), tokenLabel.size());
		}
		if (token->getTokenSerial(tokenSerial) && tokenSerial.size() <= sizeof(paddedTokenSerial))
		{
			strncpy((char*)paddedTokenSerial, (char*)tokenSerial.byte_str(), tokenSerial.size());
		}

		if (serial != NULL && label == NULL &&
			memcmp(paddedTokenSerial, paddedSerial, sizeof(paddedSerial)) == 0)
		{
			printf("Found token (%s) with matching serial.\n", i->c_str());
			tokendir = i->c_str();
			counter++;
		}
		if (serial == NULL && label != NULL &&
			memcmp(paddedTokenLabel, paddedLabel, sizeof(paddedLabel)) == 0)
		{
			printf("Found token (%s) with matching token label.\n", i->c_str());
			tokendir = i->c_str();
			counter++;
		}
		if (serial != NULL && label != NULL &&
			memcmp(paddedTokenSerial, paddedSerial, sizeof(paddedSerial)) == 0 &&
			memcmp(paddedTokenLabel, paddedLabel, sizeof(paddedLabel)) == 0)
		{
			printf("Found token (%s) with matching serial and token label.\n", i->c_str());
			tokendir = i->c_str();
			counter++;
		}

		delete token;
	}

	if (counter == 1) return true;
	if (counter > 1)
	{
		fprintf(stderr, "ERROR: Found multiple matching tokens.\n");
		return false;
	}

	fprintf(stderr, "ERROR: Could not find a token using --serial or --token.\n");
	return false;
}

// Delete a directory
bool rmdir(std::string path)
{
	bool rv = true;

#ifndef _WIN32
	// Enumerate the directory
	DIR* dir = opendir(path.c_str());

	if (dir == NULL)
	{
		fprintf(stderr, "ERROR: Failed to open directory %s", path.c_str());
		return false;
	}

	// Enumerate the directory
	struct dirent* entry = NULL;

	while ((entry = readdir(dir)) != NULL)
	{
		bool handled = false;

		// Check if this is the . or .. entry
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
		{
			continue;
		}

		// Convert the name of the entry to a C++ string
		std::string name(entry->d_name);
		std::string fullPath = path + OS_PATHSEP + name;

#if defined(_DIRENT_HAVE_D_TYPE) && defined(_BSD_SOURCE)
		// Determine the type of the entry
		switch (entry->d_type)
		{
		case DT_DIR:
			// This is a directory
			rv = rmdir(fullPath);
			handled = true;
			break;
		case DT_REG:
			// This is a regular file
			rv = rm(fullPath);
			handled = true;
			break;
		default:
			break;
		}
#endif

		if (rv == false)
			break;

		if (!handled)
		{
			// The entry type has to be determined using lstat
			struct stat entryStatus;

			if (!lstat(fullPath.c_str(), &entryStatus))
			{
				if (S_ISDIR(entryStatus.st_mode))
				{
					// This is a directory
					rv = rmdir(fullPath);
				}
				else if (S_ISREG(entryStatus.st_mode))
				{
					// This is a regular file
					rv = rm(fullPath);
				}
			}

			if (rv == false)
				break;
		}
	}

	// Close the directory
	closedir(dir);
#else
	// Enumerate the directory
	std::string pattern;
	intptr_t h;
	struct _finddata_t fi;

	if ((path.back() == '/') || (path.back() == '\\'))
		pattern = path + "*";
	else
		pattern = path + "/*";
	memset(&fi, 0, sizeof(fi));
	h = _findfirst(pattern.c_str(), &fi);
	if (h == -1)
	{
		// empty directory
		if (errno == ENOENT)
			goto finished;

		fprintf(stderr, "ERROR: Failed to open directory %s", path.c_str());

		return false;
	}

	// scan files & subdirs
	do
	{
		// Check if this is the . or .. entry
		if (!strcmp(fi.name, ".") || !strcmp(fi.name, ".."))
			continue;

		std::string fullPath = path + OS_PATHSEP + fi.name;
		if ((fi.attrib & _A_SUBDIR) == 0)
		{
			// This is a regular file
			rv = rm(fullPath);
		}
		else
		{
			// This is a directory
			rv = rmdir(fullPath);
		}

		memset(&fi, 0, sizeof(fi));

		if (rv == false)
			break;
	} while (_findnext(h, &fi) == 0);

	(void)_findclose(h);

finished:
#endif

	if (rv == false)
		return false;

	int result;
#ifndef _WIN32
	result = ::rmdir(path.c_str());
#else
	result = _rmdir(path.c_str());
#endif

	if (result != 0)
	{
		fprintf(stderr, "ERROR: Could not delete the directory: %s\n", path.c_str());
		return false;
	}

	return true;
}

// Delete a file
bool rm(std::string path)
{
	int result;

#ifndef _WIN32
	result = ::remove(path.c_str());
#else
	result = _unlink(path.c_str());
#endif

	if (result != 0)
	{
		fprintf(stderr, "ERROR: Could not delete the file: %s\n", path.c_str());
		return false;
	}

	return true;
}