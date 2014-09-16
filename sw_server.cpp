#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cstdlib>
#include <memory.h>
#include <event2/event.h>

using namespace std;

#define MAXLINE 100 
#define LISTENQ 20

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

char line[MAXLINE];

int main(int argc, char* argv[])
{
    int i, listenfd, connfd, sockfd,epfd,nfds, portnumber;
    ssize_t n;
    socklen_t clilen;

    if ( 2 == argc )
    {
        if( (portnumber = atoi(argv[1])) < 0 )
        {
            fprintf(stderr,"Usage:%s portnumber/a/n",argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr,"Usage:%s portnumber/a/n",argv[0]);
        return 1;
    }

    struct epoll_event ev, events[20];
    epfd=epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    setnonblocking(listenfd);

    ev.data.fd=listenfd;
    ev.events=EPOLLIN|EPOLLET;

    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    char *local_addr="127.0.0.1";
    inet_aton(local_addr,&(serveraddr.sin_addr));
    serveraddr.sin_port=htons(portnumber);
    bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);

    for ( ; ; ) 
	{
        nfds=epoll_wait(epfd,events,20,500);

        for(i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)
            {
                connfd = accept(listenfd,(struct sockaddr *)&clientaddr, &clilen);
				printf("connfd val is: %d\n", connfd);

                if(connfd<0)
				{
                    perror("connfd<0");
                    exit(1);
                }

                setnonblocking(connfd);

                char *str = inet_ntoa(clientaddr.sin_addr);
                cout << "accept a connection from " << str << endl;

                ev.data.fd=connfd;
                ev.events=EPOLLIN|EPOLLET;

                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
            else if(events[i].events&EPOLLIN)
            {
                if ( (sockfd = events[i].data.fd) < 0)
                    continue;

                if ( (n = read(sockfd, line, MAXLINE)) < 0) 
				{
                    if (errno == ECONNRESET) 
					{
                        close(sockfd);
                        events[i].data.fd = -1;
                    } 
					else
                        std::cout<<"readline error"<<std::endl;
                } 
				else if (n == 0) 
				{
                    close(sockfd);
                    events[i].data.fd = -1;
                }

                //line[n] = '\0';
                cout << "read: " << "size: " << n << " content: " << line << endl;

                ev.data.fd=sockfd;
                ev.events=EPOLLOUT|EPOLLET;

                //epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

            }
            else if(events[i].events&EPOLLOUT)
            {
                sockfd = events[i].data.fd;
                write(sockfd, line, n);
                ev.data.fd=sockfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
        }
    }
    return 0;
}
