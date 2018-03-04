#include <iostream>
#include "com.hpp"

std::string default_handler(std::string& request, udp::endpoint remote_endpoint_){
    return "pong\n";
}


int main(){
    try{
        boost::asio::io_service io_service;
        udp_server server(io_service, 10113, default_handler);
        io_service.run();
    }
    catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

