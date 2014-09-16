all: sw_client sw_server sw_server_libevent

sw_client:	sw_client.cpp
	g++ sw_client.cpp -o sw_client

sw_server:	sw_server.cpp
	g++ sw_server.cpp -o sw_server

sw_server_libevent: sw_server_libevent.cpp
	g++ -levent -fpermissive sw_server_libevent.cpp -o sw_server_libevent
