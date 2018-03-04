#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

void udp_sendmsg(std::string str_val, std::string host);
