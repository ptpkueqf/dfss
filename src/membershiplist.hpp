#include <iostream>
#include <boost/date_time.hpp>
#include <time.h>

class member_entry{
	public:
	std::string ip_addr_;
	time_t time_stamp_;
	bool failed_;
	member_entry(std::string ip, clock_t time_stamp){
		copy(ip.begin(), ip.end() , ip_addr_.begin());
		time_stamp_ = time_stamp;
		failed_ = false;
	}


};
