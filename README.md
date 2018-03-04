goSDFS:
requirement:
- Tolerant upto 2 machine failures at a time, make sure data is re-replicated quickly .


HOW TO BUILD:
run make in the src folder to build    
make clean to clean   

HOW TO USE
* run client INTRODUCER IP -i on introducer   
* run client INTRODUCER on the other nodes   
press 3 to execute sdfs commands    

features:   
- flat file system     
- put localfilename sdfdfilename 
- get sdfsfilename localfilename  
- ls sdfsfilenames   
- comprehensive updates  
- Updates to a given file must follow the order   
- Prompt when updates are within a minute of each other   
- wipe data after failure

Things to do:
- UPDATE MP2: Write MP2 membership list into a file so that another process can read it  
- Understand Quorum for W/W Read  
- How we decide where to store file  
- Should we store entire files ?  
- leader election - we can use the method discussed in class today  




File operations:

put localfilename sdfsfilename
get sdfsfilename localfilename
delete sdfsfilename
Ls sdfsfilename: list all machine (VM) addresses where this file is currently being stored
store: At any machine, list all files currently being stored at this machine.


Lavanya:
Put localfilename sdfs - command line arg parsing
Insert hashing and other functions
TCP async

Rahul:
Change MP2 write to file / socket
Leader re-election
Actually downloading/uploading data
Handling failure
Checking if 1 min has elapsed 

REQUIREMENT MATRIX

TASK					DESIGN 				DEVELOPMENT					INTEGRATED 				TESTED  
TCP Download/Upload        *    
Async TCP Server  		   *   
File Download/upload       *    
Routing of file            *   
Master re-election 		   *   
Integrating MP2			   *    
Integrating MP1			   *    
