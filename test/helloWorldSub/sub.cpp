#include <iostream>
#include <HelloWorldData_DCPS.hpp>
#include "util.h"

int main(int argc, char* argv[])
{
	dds::domain::DomainParticipant dp(0);
	dds::topic::Topic<HelloWorldData::Msg> topic(dp, "Message");
	dds::sub::Subscriber sub(dp);
	dds::sub::DataReader<HelloWorldData::Msg> dr(sub, topic);
	dds::core::cond::WaitSet ws;
	dds::sub::cond::ReadCondition rc(dr, dds::sub::status::DataState::new_data());
	ws += rc;
	while (true) {
		std::cout << "waiting..." << std::endl;
		ws.wait();
		auto samples = dr.read();
		std::for_each(samples.begin(), samples.end(), [](const dds::sub::Sample<HelloWorldData::Msg>& s) {
			std::cout << "DR: " << s.data() << std::endl;
		});
		std::cout << "reading done" << std::endl;
	}

	std::cout << "end of sub" << std::endl;
	return 0;
}