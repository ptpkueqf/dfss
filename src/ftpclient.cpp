#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include "ftplib.hpp"



void upload_file(char* server_ip, int port,  char* local_folder, char* file_name, char* new_name){
	int sockfd, portno, n, fileSize;
	struct hostent* server;
	struct sockaddr_in serv_addr;
	char buffer[256]= "put ";
	char change_fname[256] = "put ";
	strcat(change_fname, new_name);
	strcat(buffer,file_name);
	char fileSizeBuffer[256];
	DIR *dir;
	struct dirent *directory;
	server = gethostbyname(server_ip);
	dir = opendir(local_folder);
	portno = port;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//once socket is created:
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //IPV4
	serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
	serv_addr.sin_port = htons(portno); //port
	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    	syserr("can't connect to server");
    
    n = send(sockfd, change_fname, strlen(change_fname), 0);

    //end of common code

    //we wait for the server's ACK
    n = recv(sockfd, change_fname, sizeof(change_fname), 0);
    // if(n < 0)
    //     printf("Server didn't acknowledge name");

    //parse the string
    int j = 0;
    for(int i = 4; i <= strlen(buffer); i++)
    {
	    buffer[j] = buffer[i];
	    j++;
    }
    char address[256];
    strcpy(address,local_folder);
    strcat(address, buffer); //get file path

    //open file path
    FILE* fp;
    fp = fopen(address, "rb"); //filename, read bytes
    // if(fp == NULL)
	   //  printf("error opening file in: %s\n", buffer);
    // printf("File opened successfully!\n");

    //we will attempt to read the file
    //in chunks of 256 bytes and send!

    //figure out file size:
    int file_size = 0;
    if(fseek(fp, 0, SEEK_END) != 0)
			printf("");

    file_size = ftell(fp);
    rewind(fp);
    //printf("File size: %lu bytes\n", file_size);
    
    //pass this size to a buffer in order to send it:
    //no need for host to network long, we're passing char array
    memset(&fileSizeBuffer, 0, sizeof(fileSizeBuffer));
    sprintf(fileSizeBuffer, "%d", file_size);
    //memset(&buffer, 0, sizeof(buffer));
    //send file size:
    n = send(sockfd, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
    // if(n < 0)
	   //  printf("Error sending file size information\n"); 
    
    //receive ACK for file size:
    //give enough time for server to get
    //file size we just sent
    n = recv(sockfd, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
    // if(n < 0)
    //         printf("Error receiving handshake");
    
    //we create a byte array:
    char byteArray[256];
    memset(&byteArray, 0, sizeof(byteArray));
 
    int buffRead = 0;
    int bytesRemaining = file_size;

    //while there are still bytes to be sent:
    while(bytesRemaining != 0)
    {
         //we fill in the byte array
         //with slabs smaller than 256 bytes:
         if(bytesRemaining < 256)
         {
             buffRead = fread(byteArray, 1, bytesRemaining, fp);
             bytesRemaining = bytesRemaining - buffRead;
             n = send(sockfd, byteArray, 256, 0);
             // if(n < 0)
             //         printf("Error sending small slab\n");

             // printf("sent %d slab\n", buffRead);
         }
         //for slabs of 256 bytes:
         else
         {
             buffRead = fread(byteArray, 1, 256, fp);
             bytesRemaining = bytesRemaining - buffRead;
             n = send(sockfd, byteArray, 256, 0);
             // if(n < 0)
             //         printf("Error sending slab\n");
             // printf("sent %d slab\n", buffRead);
         }
    }
    //printf("File sent!\n");
    //clean buffers
    printf("\n[FTP-UPLOADER] upload %s completed \n", file_name);
    memset(&buffer, 0, sizeof(buffer));
    memset(&byteArray, 0, sizeof(byteArray));
    close(sockfd);

}


void ls_local(char *local_folder, std::vector<std::string>& v){
    DIR *dir;
    struct dirent *directory;
    dir = opendir(local_folder);
    if(dir){
        while((directory = readdir(dir)) != NULL){
            if(strcmp(directory->d_name, ".") == 0 ||            
             strcmp(directory->d_name, "..") == 0){
              //printf("\n%s", directory->d_name);
               // v.push_back(std::string(directory->d_name));    
            }               
        else  
            v.push_back(std::string(directory->d_name));
      }
      //rewind
      rewinddir(dir);
    }
}
void list_remote_files(char* server_ip,int port, std::vector<std::string>& files){
	int sockfd, portno, n, fileSize;
	struct hostent* server;
	struct sockaddr_in serv_addr;
	char buffer[256]= "ls-remote";
	char fileSizeBuffer[256];
	server = gethostbyname(server_ip);
	portno = port;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//once socket is created:
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //IPV4
	serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
	serv_addr.sin_port = htons(portno); //port
	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    	syserr("can't connect to server");
    //printf("Conected to ftp server \n");

    n = send(sockfd, buffer, strlen(buffer), 0);
    n = recv(sockfd, buffer, sizeof(buffer), 0);

	if(n < 0) //couldn't receive
		syserr("can't receive from server");

	//printf("running ls-remote function: %s\n", buffer);
  std::string s(buffer);
  //std::cout << s << std::endl;
  boost::split(files,s,boost::is_any_of(";"));
		 //clean buffer
  files.erase(files.end()-1);
	memset(&buffer, 0, sizeof(buffer));
	close(sockfd);
  
}

void download_file(char* server_ip, int port,  char* local_folder, char* file_name, char *new_name){
	int sockfd, portno, n, fileSize;
	struct hostent* server;
	struct sockaddr_in serv_addr;
	char buffer[256]= "get ";
	strcat(buffer,file_name);
	char fileSizeBuffer[256];
	DIR *dir;
	struct dirent *directory;
	server = gethostbyname(server_ip);
	dir = opendir(local_folder);
	portno = port;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//once socket is created:
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //IPV4
	serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
	serv_addr.sin_port = htons(portno); //port
  //std::cout << "here 1\n";
	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    	syserr("can't connect to server");
    //printf("Conected to ftp server \n");

    n = send(sockfd, buffer, strlen(buffer), 0);

    //end of common code

    //printf("User requested a download.\n");

    //we catch the file name
    char fileName[256];
    memset(&fileName, 0, sizeof(fileName));
    
    //parse
    int j = 0;
    for(int i = 4; i <= strlen(buffer); i++)
    {
	    fileName[j] = buffer[i];
	    j++;
    }

    //catch file size:
    recv(sockfd, buffer, sizeof(buffer), 0);
    fileSize = atoi(buffer);

    //send size back as ACK:
    send(sockfd, buffer, sizeof(buffer), 0);

    //print file name and size:
    //printf("File: '%s' (%d bytes)\n",fileName, fileSize);
    

    //receive data:
    memset(&buffer, 0, sizeof(buffer));
    int remainingData = 0;
    ssize_t len;
    char path[256]; //= "./folder-local/";
    strcpy(path,local_folder);
    strcat(path, new_name);
    //printf("path: %s", path);
    //std::cout << "here " << path << std::endl;
    FILE* fp;
    fp = fopen(path, "wb");//overwrite if existing
    							//create if not
    remainingData = fileSize;
    //while(((len = recv(sockfd, buffer, 256, 0)) > 0) && (remainingData > 0))
    //printf("remainingData: %d", remainingData);
    while(remainingData != 0)
    {
	    if(remainingData < 256)
	    {
		    len = recv(sockfd, buffer, remainingData, 0);
		    fwrite(buffer, sizeof(char), len, fp);
		    remainingData -= len;
		    //printf("Received %lu bytes, expecting %d bytes\n", len, remainingData);
		    break;
	    }
	    else
	    {
	    	len = recv(sockfd, buffer, 256, 0); //256
	    	fwrite(buffer, sizeof(char), len, fp);
        remainingData -= len;
	    	//printf("Received %lu bytes, expecting: %d bytes\n", len, remainingData);
	    }
    }
    printf("\n[FTP-DOWNLOADER] download %s completed \n", file_name);
    fclose(fp);
    //n = recv(sockfd, buffer, 256, 0); //receive bizarre lingering packet.

    //clean buffer
    memset(&buffer, 0, sizeof(buffer));
    close(sockfd);
}

void delete_file(std::string file_name){

  if( unlink((char *) file_name.c_str() ) != 0 )
    perror( "Error deleting file" );
  else
  std::cout <<  "File " << file_name << " successfully deleted from Server \n" ;
}



