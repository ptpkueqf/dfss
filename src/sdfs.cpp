#include <iostream>
#include "com.hpp"
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>
#include <string>
#include <boost/thread.hpp>
#define UP_RECV 2565

/*
 * @returns true if file exists
 * A function to check if file exists in local file system
 */
inline bool is_file_exist(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}


/*
 * Function to get the size of the file
 */
std::ifstream::pos_type get_filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}

void upload_receiver(){

    boost::asio::io_service io_service;
    tcp_file_server server(io_service, UP_RECV);
    std::cout << "[DOWNLOAD SERVER] Started    \n";
    io_service.run();	
}

int main(void){
	//have a thread for downloader
	boost::thread t(upload_receiver);
	while(1){
		
		std::cout << "Enter filename to upload a file \n" ;
		file_meta_data fm;
		std::cin >> fm.file_name;
		if(!is_file_exist(fm.file_name)){
			std::cout << "Enter valid filename \n";
			continue;
		}
		fm.file_size = get_filesize(fm.file_name);
		std::string ip_addr = "10.194.79.80";
		std::string request = "20";
		std::string response;
		std::string new_file_name = "new";
		tcp_send_file(ip_addr, UP_RECV, new_file_name , fm,response);
		std::cout << response << std::endl;

		std::cout << "The  file size : " << fm.file_size << std::endl;
	}

}