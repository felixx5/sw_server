#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

const char* HOST = "127.0.0.1";
const int PORT = 10086;

int init()
{
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd <= 0)
	{
		printf("create client socket error %s\n", strerror(errno));
		return -1;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, HOST, &server_addr.sin_addr) <= 0)
	{
		printf("inet_pton error for %s, errno:%s\n", HOST, strerror(errno)); 
		return -1;
	}

	int connect_res = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (connect_res < 0)
	{
		printf("connect failed for %s, connect_res %d, errno:%s\n", HOST, connect_res, strerror(errno));
		return -1;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	int sock_fd;
	if ((sock_fd = init()) < 0)
	{
		printf("init failed");
		return 0;
	}

	const char* msg = "this is not a hello world.";
	printf("ryf send: %s %d \n", msg, strlen(msg));
	send(sock_fd, msg, strlen(msg), 0);

	sleep(3000);
	
	return 0;
}

