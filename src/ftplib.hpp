void upload_file(char* server_ip, int port,  char* local_folder, char* file_name, char* new_name);
void ls_local(char *local_folder, std::vector<std::string>& v);
void list_remote_files(char* server_ip,int port, std::vector<std::string>& files);
void download_file(char* server_ip, int port,  char* local_folder, char* file_name, char *new_name);
int file_server(int port_no, const char* folder);
void delete_file(std::string file_name);


#ifndef SYSERR_H
#define SYSERR_H

void syserr(char* msg);
#endif 