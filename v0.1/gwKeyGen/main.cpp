#include <iostream>
#include "library.h"

using namespace std;

int main(int argc, char* argv[])
{
	void* module;
	CK_FUNCTION_LIST_PTR p11 = NULL;
	if (loadLib(&module, &p11) == -1) {
		cout << "ERROR: loadLib" << endl;
		return -1;
	}
	cout << "loadLib ok" << endl;

	unloadLib(module);
	cout << "end" << endl;
	return 0;
}