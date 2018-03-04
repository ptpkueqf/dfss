/*
 * This the main part of the failure detector and File System which runs on every node. 
 * Runs the following background threads
 * -> Servo - listens to heartbeats from other nodes
 * -> heartbeater - sends heartbeat to other nodes
 * -> updater - checks for updates
 * -> failure detector , checks for active failures
 */


/*
 * Folder Info 
 * downloads - downloaded from get 
 * uploads - puts
 */
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "com.hpp"
#include <boost/algorithm/string.hpp>
#include <map>
#include "membershiplist.hpp"
#include <boost/thread.hpp>
#include "virtual_ring.hpp"
#include <cstdio>
#include <time.h>
#include "ftplib.hpp"
#include <mutex>
#include <boost/date_time.hpp>
#define MEM_LISTNER_PORT 10114
#define INTRO_PORT 10115
#define UPDATE_PORT 10116
#define DEBUG 0     
#define PUTFILE_PORT 10117
#define FTP_SERVER_PORT 10118
#define MONITOR_SERVER_PORT 10119
using boost::asio::ip::udp;


std::map<std::string,member_entry> member_list;
std::vector<std::string> local_neighbour_list;
boost::mutex local_neighbour_list_lock;
boost::mutex member_list_lock;
virtual_ring *ring;
std::string master;

/*
 * This is the function which writes to the log file on the node
 */
void write_log(std::string str){

  std::ofstream ofs;
  ofs.open ("../../log", std::ofstream::out | std::ofstream::app);

  ofs << str << std::endl;

  ofs.close();
}

void start_ftp_server(const char *directory){
    try{
        std::cout << "[FTP-SERVER] Started\n";
        file_server(FTP_SERVER_PORT, directory);
    }catch(...){
        std::cout << "[FTP-SERVER] Stopped\n";
        start_ftp_server(directory);
    }
}

/*
 * This is a default handler
 * @param string request 
 * @param string r_ep
 */
std::string default_handler(std::string& request , udp::endpoint r_ep){
    return "pong\n";

}


/*
 * This is a default handler
 * @param string ip
 */
void add_to_memlist(std::string& ip){
    member_list_lock.lock();
    time_t add_time;
    time(&add_time);
    if(member_list.find(ip) == member_list.end()){
        member_entry m_ent(ip, add_time);
        member_list.insert(std::pair<std::string, member_entry>(ip , m_ent));   
    }
    member_list_lock.unlock();
    
}



/*
 * Function to add node into local neighbour list
 * @param string ip_addr
 */
void add_to_local_neighbour_list(std::string& ip_addr ){
    local_neighbour_list_lock.lock();
    if(local_neighbour_list.size() < 4){
        local_neighbour_list.push_back(ip_addr);
    }
    local_neighbour_list_lock.unlock();
}


/*
 * this function receives the hearbeat from the neighbour
 * @param string request 
 * @param string r_ep
 */
std::string heartbeat_handler(std::string& request, udp::endpoint r_ep){
    member_list_lock.lock();
    std::string ip_addr = r_ep.address().to_string() ;
    std::map<std::string,member_entry>::iterator it = member_list.find(ip_addr); 
    if(it != member_list.end()){
        if(DEBUG)
            std::cout << "The node exists in the member list \n";
        time_t add_time;
        time(&add_time);
        it->second.time_stamp_ = add_time;//clock();
        it->second.failed_ = false;
        add_to_local_neighbour_list(ip_addr);

    }else{
        if(DEBUG)
            std::cout << "The node does not exist in the member list \n";
        time_t add_time;
        time(&add_time);
        //std::cout << "The time is " << add_time << std::endl;
        member_entry m_ent(ip_addr, add_time);
        member_list.insert(std::pair<std::string, member_entry>(ip_addr , m_ent)); 
    }
    //std::cout << request << std::endl;
    std::vector<std::string> vs1;
    boost::split(vs1, request , boost::is_any_of(";"));
    for(int i=0; i < vs1.size()-1; ++i ){
        std::vector<std::string> temp;
        boost::split(temp, vs1[i] , boost::is_any_of(":"));
        //std::cout << "AFTER: " << temp[0] << "  " << temp[1] << std::endl;
        std::map<std::string,member_entry>::iterator it = member_list.find(temp[0]);
        if(it != member_list.end()){
            
            if(it->second.failed_ == true && temp[1] == "0"){
                it->second.failed_ = false;
            }
            local_neighbour_list_lock.lock();
            if((it->second.failed_ == false) && temp[1] == "1" && (find(local_neighbour_list.begin(),local_neighbour_list.end(), it->first) == local_neighbour_list.end()))
            {
                //write_log(it->first + "Has failed ");
                it->second.failed_ = true;
            }
            local_neighbour_list_lock.unlock();

        }else{
            time_t add_time;
            time(&add_time);
            member_entry m_ent(temp[0], add_time);
            member_list.insert(std::pair<std::string, member_entry>(temp[0] , m_ent)); 
        } 
    }
    member_list_lock.unlock();
    return "pong";
}

/*
 * This is a default handler
 * @param string request 
 * @param string message
 */
void runner_back(std::string remote_ip, std::string message){
    std::string response;
    try{
        udp_sendmsg(message, remote_ip, UPDATE_PORT, response);
    }catch(boost::system::system_error const& e){
    
    }
   
}

/*
 * updates the node
 * @param string request 
 * @param string r_ep
 */
void update_nodes(std::string ip){
    node* t = ring->introducer;
    //std::cout << "In update nodes\n";
    for(int i=0; i < ring->count ; ++i ){
        std::string response = "";
        std::set<std::string> ip_s;
        if(t->next1 != NULL){
            if(t->ip_addr.compare(t->next1->ip_addr) != 0){
                ip_s.insert(t->next1->ip_addr);           
            }

        }
        if(t->next2 != NULL){
            if(t->ip_addr.compare(t->next2->ip_addr) != 0){
                ip_s.insert(t->next2->ip_addr);           
            }
        }
        if(t->prev1 != NULL){
            if(t->ip_addr.compare(t->prev1->ip_addr) != 0){
                ip_s.insert(t->prev1->ip_addr);           
            }
        }
        if(t->prev2 != NULL){
            if(t->ip_addr.compare(t->prev2->ip_addr) != 0){
                ip_s.insert(t->prev2->ip_addr);           
            }
        }
        for(std::set<std::string>::iterator it = ip_s.begin(); it != ip_s.end(); it++){
            response += (*it);
            response += ";";
        }
       if(t->ip_addr != ring->introducer->ip_addr ){
            std::string ip(t->ip_addr);
            boost::thread t(runner_back, ip , response );
            
       }
       t = t->next1;

    }
}

/*
 * This is a handler for the intro
 * @param string request 
 * @param endpoint r_ep
 */
std::string intro_handler(std::string& request, udp::endpoint r_ep){
    //std::cout << "The request is " << request << std::endl;
    std::string ip = r_ep.address().to_string();
    //std::cout << "The IP ADDRESS IS " << ip << std::endl;
    if(request == "JOIN"){

        node* t = ring->insert(ip);
        ring->display();
        std::string response = "";
        std::set<std::string> ip_s;
        if(t->next1 != NULL){
            if(ip.compare(t->next1->ip_addr) != 0){
                ip_s.insert(t->next1->ip_addr);           
            }

        }
        if(t->next2 != NULL){
            if(ip.compare(t->next2->ip_addr) != 0){
                ip_s.insert(t->next2->ip_addr);           
            }
        }
        if(t->prev1 != NULL){
            if(ip.compare(t->prev1->ip_addr) != 0){
                ip_s.insert(t->prev1->ip_addr);           
            }
        }
        if(t->prev2 != NULL){
            if(ip.compare(t->prev2->ip_addr) != 0){
                ip_s.insert(t->prev2->ip_addr);           
            }
        }
        for(std::set<std::string>::iterator it = ip_s.begin(); it != ip_s.end(); it++){
            response += (*it);
            response += ";";
        }
        boost::thread thr( update_nodes, ip);
       
        return response;
    }
    if(request == "LEAVE"){
        ring->leave(ip);
        ring->display();
        boost::thread thr( update_nodes, ip);
        return "pong";
    }
    return "pong";

}

/*
 * Background process which does master election
 *
 */
void master_elector(){
    std::map<std::string,member_entry>::iterator it = member_list.end();
    if(member_list.size() > 0){
        it--;
        while(it != member_list.begin()){
            if(it->second.failed_ == false){
                if(master != it->first){
                        master = it->first;
                        std::cout << "New master elected " << master << std::endl;
                            
                }
                break;
            }
            it--;
        }   
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    master_elector();


}
int number_of_active_nodes(){
    std::map<std::string,member_entry>::iterator it = member_list.begin();
    int count = 0;
    while(it != member_list.end()){
        if(it->second.failed_ == false)
            count++;
        it++;
    }
    return count;
}

std::vector<std::string> where_to(std::string request){
    int val = 37+ ((int(request.at(0)) -'a') % (number_of_active_nodes()-1));
    std::map<std::string,member_entry>::iterator it = member_list.begin();
    std::vector<std::string> res;
    while(it != member_list.end()){
        std::vector<std::string> strs;
        boost::split(strs,it->first,boost::is_any_of("."));
        int val1 = atoi(strs[3].c_str());
        if(val1 >= val && it->second.failed_ == false){
            res.push_back(it->first);
            std::map<std::string,member_entry>::iterator it2 = ++it;
            it--;
            while(it2->second.failed_ == true && it2 != member_list.end())
                it2++;
            if(it2 == member_list.end()){
                it2 = member_list.begin();
                while(it2->second.failed_ == true && it2 != member_list.end())
                    it2++;
                res.push_back(it2->first);
            }else{
                res.push_back((it2)->first);
            }
            if(it == member_list.begin())
                it2 = member_list.begin();
            else
                it2 = --it;
                it++;
            
            while(it2->second.failed_ == true && it2 != member_list.begin()){
                it2--;
            }
            if(it2 == member_list.begin()){
                it2 = member_list.end();

                while(it2->second.failed_ == true && it2 != member_list.begin())
                    it2--;
                res.push_back(it2->first);
            }else{
                res.push_back((it2)->first);
            }
            return res;
        }
        //std::cout << count++ << " \t\t\t" << it->first << " \t\t\t" << boolstr( it->second.failed_)  << std::endl;
        it++;
    } 
    res.push_back(member_list.end()->first);
    res.push_back(member_list.begin()->first);
    std::map<std::string,member_entry>::iterator third = member_list.begin();
    third++;
    res.push_back(third->first);

    return res; 
}

/*
 * Runs on every single node and basically tells if the file is present or deletes file
 */
std::string monitor_handler(std::string& request, udp::endpoint r_ep){
    //std::vector<std::string> params;
    //boost::split(params,request,boost::is_any_of(";"));
    std::vector<std::string> arr;
    boost::split(arr,request,boost::is_any_of(";"));
    if(arr[1] == "DELETE"){
        delete_file("./uploads/" + arr[0]);
        return "YES;" +  r_ep.address().to_string();
    }else{
        std::vector<std::string> local_file_list;
        ls_local("./uploads/", local_file_list);
        for(int i=0; i < local_file_list.size(); ++i){
            if(local_file_list[i] == arr[0]){
                //std::cout << "found file in server \n";
                return "YES;" + r_ep.address().to_string();
            }
        }
        return "NO;" +  r_ep.address().to_string();
    }
    

}

std::map<std::string,time_t> safety_check;
/*
 * Handler for putfile server
 * basically tells where all files are present
 */
std::string putfile_handler(std::string& request, udp::endpoint r_ep){
    std::vector<std::string> params;
    boost::split(params,request,boost::is_any_of(";"));
    int force = stoi(params[0]);
    if(force == 0){
        time_t timer;
        time(&timer);    
        if(safety_check.find(params[1]) != safety_check.end()){
            time_t seconds = timer - safety_check[params[1]];
            if(seconds < 60){
                return "CONFLICT";
            }
        }
        safety_check[params[1]] = timer;
    }
    std::vector<std::string> ips = where_to(params[1]);//ring->put_file(request);
    std::string ip;
    for(int i =0; i < ips.size(); ++i){
        ip += ips[i] + ";";
    }
    //std::cout << ip;
    return ip;   


}



/*
 * This is the listener which listens to heartbeat from neighbours
 *
 */
void servo(){
    try{

        boost::asio::io_service io_service;
        udp_server server(io_service, MEM_LISTNER_PORT, heartbeat_handler);
        std::cout << "[SERVO] Started    \n";
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

/*
 * This is the thread run by the introducer
 */
void intro(){
    std::cout << "[INTROX] This node is running as the introducer." << std::endl;
    try{

        boost::asio::io_service io_service;
        udp_server server(io_service, INTRO_PORT, intro_handler);
        std::cout << "[INTROX] Started    \n";
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }



}

/*
 * This is a handler for the update task
 * @param string request 
 * @param endpoint r_ep
 */
std::string update_handler(std::string& response, udp::endpoint r_ep){
    // std::cout << "IN update handler " << response << "\n";
    local_neighbour_list_lock.lock();
    boost::split(local_neighbour_list, response , boost::is_any_of(";"));
    if(local_neighbour_list[local_neighbour_list.size()-1].compare("") == 0)
    {
       // std::cout << "here1\n" ;
        local_neighbour_list.erase(local_neighbour_list.begin() + local_neighbour_list.size() -1 );
    }
    //write_log("New Node has joined the system,will be updated in memberlist soon.");
    //std::cout << "here3\n" ;
    local_neighbour_list_lock.unlock();
    //std::cout << "here4\n" ;
    return "pong";
}

void updater(){
    try{
        boost::asio::io_service io_service;
        udp_server server(io_service, UPDATE_PORT, update_handler);
        io_service.run();

    }catch(std::exception e){
        std::cerr << e.what() << std::endl;
    }
}

void contact_introducer(const std::string& introducer_ip){
    std::string response;
    udp_sendmsg("JOIN",introducer_ip,INTRO_PORT, response);
    local_neighbour_list_lock.lock();
    boost::split(local_neighbour_list, response , boost::is_any_of(";"));
    if(local_neighbour_list[local_neighbour_list.size()-1].compare("") == 0)
    {
        local_neighbour_list.erase(local_neighbour_list.begin() + local_neighbour_list.size() -1 );
    }
    local_neighbour_list_lock.unlock();
}

void leave_introducer(const std::string& introducer_ip){
    std::string response;
    udp_sendmsg("LEAVE",introducer_ip,INTRO_PORT, response);
}

void runner(std::string remote_ip, std::string message){
    std::string response;
    //std::cout << "Querying " << server_ip << std::endl;
    try{
        udp_sendmsg(message, remote_ip, MEM_LISTNER_PORT, response);
        //res = send_request(server_ip , DEFAULT_SERVER_PORT , query , response ); 
    }catch(boost::system::system_error const& e){
        //std::cout << "Could not connect : " << e.what() << std::endl;
    }
   
}
std::string bool_to_string(bool b)
{
  return b ? "1" : "0";
}

std::string boolstr(bool a){
    return (a ? "Failed" : "Alive");
}

/*
 * serializes the member list into a string
 */
void make_message(std::string& str){
    std::map<std::string,member_entry>::iterator it = member_list.begin();
    while(it != member_list.end()){
        str += it->first + ":" + bool_to_string(it->second.failed_) + ";";
        it++;
    }
    
}

/*
 * Thread which constantly heartbeats to neighbour nodes
 *
 */
void heartbeater(bool is_introx){
    while(1){
        if(!is_introx){

            // std::vector<boost::thread*> threads(local_neighbour_list.size()-1);
            // boost::thread_group g;
            local_neighbour_list_lock.lock();
            for(int i=0; i < local_neighbour_list.size() ; ++i ){
                std::string response;
                // std::cout << "Contacting " << local_neighbour_list[i] << std::endl;
                std::string temp(local_neighbour_list[i]);
                //copy(local_neighbour_list[i]->begin(),local_neighbour_list->end(),temp.begin());
                std::string message;
                make_message(message);
                std::map<std::string,member_entry>::iterator it = member_list.find(temp);
                if(it->second.failed_ != true)
                    boost::thread t(runner, temp , message );
                // threads[i] = new boost::thread(runner, local_neighbour_list[i] , "query");
                // g.add_thread(threads[i]);
                //udp_sendmsg("MEMBERLIST",clean_string(local_neighbour_list[i]), MEM_LISTNER_PORT ,response );

            }
            local_neighbour_list_lock.unlock();

        }

        boost::this_thread::sleep(boost::posix_time::milliseconds(300));

    }
}

bool in_local(std::string ip){
    local_neighbour_list_lock.lock();
    for(int i=0; i < local_neighbour_list.size(); ++i){
        if(local_neighbour_list[i].compare(ip) == 0 ){

            local_neighbour_list_lock.unlock();
            return true;
        }
    }
    local_neighbour_list_lock.unlock();
    return false;
}

/*
 * Thread which checks for failure
 */
void failure_detector(){
    while(1){
        local_neighbour_list_lock.lock();
        for(int i=0; i < local_neighbour_list.size() ; ++i){
            std::map<std::string,member_entry>::iterator it = member_list.find(local_neighbour_list[i]);
            if(it != member_list.end()){
                time_t timer;
                time(&timer);
                time_t seconds = timer - it->second.time_stamp_;
                if(seconds >= 2   ){
                    it->second.failed_ = true;
                    //write_log("Detecting here that "+it->first + "Has failed ");
                }
            }
            
        }
        local_neighbour_list_lock.unlock();
        boost::this_thread::sleep(boost::posix_time::milliseconds(250));
    }
}

void putfile_listener(){
    
    try{

        boost::asio::io_service io_service;
        udp_server server(io_service, PUTFILE_PORT, putfile_handler);
        std::cout << "[PFILE LISTENER] started" << std::endl;
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }




}
void monitor_listener(){
    
    try{
        boost::asio::io_service io_service;
        udp_server server(io_service, MONITOR_SERVER_PORT, monitor_handler);
        std::cout << "[MONITOR LISTENER] started" << std::endl;
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}



void check_for_file(std::string file_name){

}

std::string find_file_remote(std::vector<std::string> ips, std::string file_name){
    //for(int i=0; i < )
    for(int i=0; i < ips.size()-1; ++i){
        std::string file_check;
        udp_sendmsg(file_name + ";CHECK", ips[i] , MONITOR_SERVER_PORT, file_check);
        std::vector<std::string> return_vals;
        boost::split(return_vals, file_check , boost::is_any_of(";"));
        if(return_vals[0] == "YES"){
            return ips[i];
        }
    }
    return "";
}

/*
 * Thread which monitors files in local system and uploads them to other servers;
 */
void monitor(){

    boost::this_thread::sleep(boost::posix_time::milliseconds(15000));
    //std::cout << "[MONITOR] monitoring local fs\n";
    if(member_list.size() >= 4){
        std::vector<std::string> local_file_list;
        ls_local("./uploads/", local_file_list);
        for(int i=0; i < local_file_list.size() ; ++i){
            std::string response = "";
            bool file_not_required = true;
            udp_sendmsg("1;"+local_file_list[i], master, PUTFILE_PORT, response);
            std::vector<std::string> arr;
            boost::split(arr,response,boost::is_any_of(";"));   
            for(int j=0; j < arr.size()-1;++j){
                std::string file_check;

                udp_sendmsg(local_file_list[i] + ";CHECK", arr[j], MONITOR_SERVER_PORT, file_check);
                std::vector<std::string> return_vals;
                boost::split(return_vals, file_check , boost::is_any_of(";"));
                if(return_vals[1] == arr[j]){
                    //std::cout << "comparing " << return_vals[1] << " : " << arr[j] << std::endl;
                    file_not_required = false;
                }
                if(return_vals[0] == "NO"){
                    std::cout << "Replicating file " << local_file_list[i] << " on " << arr[j] << std::endl;
                    upload_file((char *)arr[j].c_str(),  FTP_SERVER_PORT , "./uploads/", (char *)local_file_list[i].c_str(),(char *) local_file_list[i].c_str());
                }
            }
            if(file_not_required){
                std::cout << "File " << local_file_list[i] << "is redundant, removing File \n" ;
                delete_file("./uploads/" + local_file_list[i]);
            }
        }
    }
   
    monitor();

}

inline bool is_file_exist(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}


int main(int argc, char* argv[])
{
    bool is_introx = false;
    boost::thread_group background_servers;

    if(argc >3 || argc < 1 ){
        std::cout    << "[USAGE] - ip-address and [OPTION] -i for introducer " << std::endl;
        return 0;
    }


    if(argc == 3 ){  
        if(strcmp(argv[2], "-i") == 0){
            ring = new virtual_ring(argv[1]);
            boost::thread intro_thread(intro);        
            background_servers.add_thread(&intro_thread);
            is_introx = true;
           
        }
    }

    if(!is_introx){
        //we need to contact the introducer node to this.
        contact_introducer(argv[1]);
    }


    boost::thread servo_thread(servo);
        // Start FTP server
    boost::thread ftp_server_thread(start_ftp_server,"./uploads/");
    background_servers.add_thread(&ftp_server_thread);
    background_servers.add_thread(&servo_thread);
    // std::string response;
    // udp_sendmsg("ping", argv[1], 10113, response);
    // std::cout << response;
    if(!is_introx){

        for(int i =0 ; i < local_neighbour_list.size(); ++i ){
            add_to_memlist(local_neighbour_list[i]);
        }        
    }                                 

    boost::thread hbt(heartbeater , is_introx);
    background_servers.add_thread(&hbt);
    boost::thread file_thread(putfile_listener);
    background_servers.add_thread(&file_thread);

    boost::thread monitor_server(monitor_listener);
    background_servers.add_thread(&monitor_server);

    boost::thread master_election(master_elector);
    background_servers.add_thread(&master_election);

    boost::thread monitor_thread(monitor);
    background_servers.add_thread(&monitor_thread);

    if(!is_introx){
        boost::thread fail_det(failure_detector);
        background_servers.add_thread(&fail_det);   
        boost::thread upd(updater);
        background_servers.add_thread(&upd);     
    }
    
    master = std::string(argv[1]);
    if(is_introx){
        while(1){
            std::map<std::string,member_entry>::iterator it;// = member_list.find(ring->introducer->next1->ip_addr);
              //  if(it->second.failed_ != true)
            if(ring->introducer->next1 != NULL)
            {
                it = member_list.find(ring->introducer->next1->ip_addr);
                if(it->second.failed_ != true )
                    boost::thread t(runner, ring->introducer->next1->ip_addr , "" );
            }
            if(ring->introducer->next2 != NULL)
            {
                it = member_list.find(ring->introducer->next2->ip_addr);
                if(it->second.failed_ != true)
                    boost::thread t(runner, ring->introducer->next2->ip_addr , "" );
            }
            if(ring->introducer->prev1 != NULL)
            {
            
                it = member_list.find(ring->introducer->prev1->ip_addr);
                if( it->second.failed_ != true)
                    boost::thread t(runner, ring->introducer->prev1->ip_addr , "" );
            }
            if(ring->introducer->prev2 != NULL)
            {
            
                it = member_list.find(ring->introducer->prev2->ip_addr);
                if( it->second.failed_ != true)
                    boost::thread t(runner, ring->introducer->prev2->ip_addr , "" );
            }
            boost::this_thread::sleep(boost::posix_time::milliseconds(300));
        }
    }




    for(int csd =0 ; csd < 10; csd++){
        int choice;
        std::cout << "PRESS 1 to leave the network\nPRESS 2 to display membership list\nPRESS 3 to enter SDFS command\n";
        try{
            std::cin >> choice;
        }catch(...){
            continue;
        }
        if(choice == 1){
            leave_introducer(argv[1]);
        }else if(choice == 2){
            std::map<std::string,member_entry>::iterator it = member_list.begin();
            std::cout << "MEMBERSHIP LIST\n---------------------------------------------------\nNode Name\t\tIP Address\t\tStatus\n";
            int count = 1;
            while(it != member_list.end()){
                std::cout << count++ << " \t\t\t" << it->first << " \t\t\t" << boolstr( it->second.failed_)  << std::endl;
                it++;
            }   
        }
        else if(choice==3){
            std::string command;
            std::cout << "Enter SDFS command \n";
            //std::cin >> command;
            std::cin.ignore();
            std::getline(std::cin , command);
            std::vector<std::string> v;
            //std::cout << "Entered command is " << command << std::endl;
            boost::split(v, command , boost::is_any_of(" "));
            //std::cout << "Size is " << v.size() << std::endl;

            if(v[0] == "put"){
                //std::cout << "In put \n";
                std::string response;
                if(v.size()!=3)
                {
                    std::cout << "Invalid Command\n";
                    continue;
                }
                if(!is_file_exist("./"+v[1])){
                    std::cout << "File does not exist on local file system\n";
                    continue; 
                }
                try{

                    udp_sendmsg("0;"+v[2], master, PUTFILE_PORT, response);
                    //std::cout << "Response is " << response << std::endl;
                    if(response == "CONFLICT"){
                        std::cout << "Are you sure(yes/no) you want to rewrite as last write was less than 30 seconds ago\n";
                        std::string choice;
                        
                        std::cin >> choice;
                        if(choice == "yes"){
                            response = "";
                            udp_sendmsg("1;"+v[2], master, PUTFILE_PORT, response);
                        }else{
                            continue;
                        }
                    }
                    std::vector<std::string> arr;
                    boost::split(arr,response,boost::is_any_of(";"));
                    for(int i=0; i < arr.size()-1; ++i){
                        std::cout << "Uploading " << v[1] << " as " << v[2] << " to " <<  arr[i] << std::endl;
                        upload_file((char *)arr[i].c_str(),  FTP_SERVER_PORT , "./", (char *)v[1].c_str(),(char *) v[2].c_str());
                    }

                }catch(...){
                    std::cout << "Upload Failed ! \n" ;
                }
            }else if(v[0] == "get"){
                std::string response;
                if(v.size()!= 3){
                    std::cout << "Invalid command\n";
                    continue;
                }
                try{
                    udp_sendmsg("1;"+v[1], master, PUTFILE_PORT, response);
                    //std::cout << "Response is " << response << std::endl;
                    std::vector<std::string> arr;
                    boost::split(arr,response,boost::is_any_of(";"));          
                    std::cout << "Downloading " << v[1] << " as " << v[2] << " from " <<  arr[0] << std::endl;
                    
                    std::string url = find_file_remote(arr,v[1]);
                    if(url == "")
                    {
                        std::cout << "File does not exist on SDFS\n";
                    }
                    else{
                        download_file((char *)url.c_str(),  FTP_SERVER_PORT , "./downloads/", (char *)v[1].c_str(),(char *) v[2].c_str());
                    }
                }catch(...){
                    std::cout << "Download Failed ! \n" ;
                    continue;
                }


            }else if(v[0] == "delete"){
                std::string response;
                if(v.size()!=2)
                {
                    std::cout << "Invalid Command\n";
                    continue;
                }
                try{

                    udp_sendmsg("1;"+v[1], master, PUTFILE_PORT, response);
                    //std::cout << "Response is " << response << std::endl;
                    std::vector<std::string> arr;
                    boost::split(arr,response,boost::is_any_of(";"));
                    for(int i=0; i < arr.size()-1; ++i){
                        std::string response;
                        udp_sendmsg(v[1] + ";DELETE", arr[i], MONITOR_SERVER_PORT, response);
                
                    }

                }catch(...){
                    std::cout << "Upload Failed ! \n" ;
                }

            }else if(v[0] == "store"){
                std::vector<std::string> v1,v2;
                ls_local("./uploads/", v1);
                ls_local("./backups/", v2);
                std::cout << "Listing Files on this node\n";
                for (int i = 0; i < v1.size(); ++i)
                {
                    std::cout << v1[i] << std::endl;
                }
                for (int i = 0; i < v2.size(); ++i)
                {
                    std::cout << v2[i] << std::endl;
                }                

            }else if(v[0] == "ls"){
                std::string response;
                try{
                    udp_sendmsg("1;" + v[1], master, PUTFILE_PORT, response);
                    //std::cout << "Response is " << response << std::endl;
                    std::vector<std::string> arr;
                    boost::split(arr,response,boost::is_any_of(";"));
                    std::string url = find_file_remote(arr,v[1]);
                    if(url == "")
                    {
                        std::cout << "File does not exist on SDFS\n";
                    }else{
                        std::cout << "File " << v[1] << " Present at :\n";
                        for(int i=0; i < arr.size()-1; ++i){
                            std::cout <<  arr[i] << std::endl;
                        }
                    }
                }catch(...){
                    std::cout << "Upload Failed ! \n" ;
                }
            }else{
                break;
            }

        }
    }

    return 0;
}

