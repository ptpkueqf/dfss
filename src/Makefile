all: ./../out/server ./../out/client 

./../out/sdfs: ./../out/com.o ./../out/sdfs.o
	mkdir -p ./../out
	g++ -std=c++11 ./../out/sdfs.o ./../out/com.o -o ./../out/sdfs -lboost_thread  -lboost_system -pthread

./../out/sdfs.o: sdfs.cpp
	mkdir -p ./../out
	g++ -std=c++11 -c sdfs.cpp -o ./../out/sdfs.o



./../out/server:  ./../out/com.o ./../out/server.o 
	mkdir  -p ./../out
	g++ -std=c++11 ./../out/server.o ./../out/com.o -o ./../out/server -lboost_system -pthread

./../out/server.o: server.cpp
	mkdir  -p ./../out
	g++ -std=c++11 -c server.cpp -o ./../out/server.o

./../out/com.o: com.hpp com.cpp 
	mkdir -p ./../out
	g++ -std=c++11 -c com.cpp -o ./../out/com.o

./../out/client:  ./../out/com.o ./../out/client.o ./../out/virtual_ring.o ./../out/ftpclientlib.o ./../out/ftpserverlib.o
	mkdir -p ./../out
	mkdir -p ./../out/downloads
	mkdir -p ./../out/backups
	mkdir -p ./../out/uploads
	g++ -std=c++11  ./../out/client.o ./../out/com.o ./../out/ftpclientlib.o ./../out/ftpserverlib.o ./../out/virtual_ring.o -o ./../out/client -lboost_thread -lboost_system -pthread


./../out/client.o: client.cpp membershiplist.hpp
	mkdir -p ./../out
	g++ -std=c++11 -w -c client.cpp  -o ./../out/client.o

./../out/virtual_ring.o: virtual_ring.cpp virtual_ring.hpp
	mkdir -p ./../out
	g++ -std=c++11 -c virtual_ring.cpp -o ./../out/virtual_ring.o

./../out/ftpclientlib.o: ftplib.hpp ftpclient.cpp 
	g++ -std=c++11 -c -w ftpclient.cpp  -o ./../out/ftpclientlib.o

./../out/ftpserverlib.o: ftplib.hpp ftpserver.cpp 
	g++ -std=c++11 -w -c ftpserver.cpp  -o ./../out/ftpserverlib.o

clean:
	rm -rf ./../out
