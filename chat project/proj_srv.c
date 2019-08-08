#include	"unp.h"
#include	<time.h>
#include        <stdbool.h>


int
main(int argc, char **argv)
{
	int i, maxi, listenfd, connfd, sockfd, maxfd;
	int nready, client[FD_SETSIZE];;
	ssize_t n;
	char buf[MAXLINE];
	fd_set allset, rset;
	socklen_t clilen;
	struct sockaddr_in	servaddr, cliaddr;

	bool con_check = false;

	FILE *fp = stdin;
	int stdineof = 0;
	
	//long arg, result;
	char recvline[MAXLINE],sendline[MAXLINE];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));	/* daytime server */
	
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);
	printf("[server address is %s : %d]\n", Inet_ntop(AF_INET, &servaddr.sin_addr, buf,sizeof(buf)), ntohs(servaddr.sin_port));

	maxfd = listenfd;
	maxi = -1;
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	FD_SET(fileno(fp), &allset); 

	printf("[waiting for a talking client ]\n");

	for(;;)
	{
		if(con_check)
			break;
		
		rset = allset;
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) 
		{
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *)&cliaddr, &clilen);
			printf("[connected to %s:%d]\n", Inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf)), ntohs(cliaddr.sin_port));
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) 
				{
					client[i] = connfd;
					break;
				}

			if (i == FD_SETSIZE)
				err_quit("too many clients");

			FD_SET(connfd, &allset);

			if (connfd > maxfd)
				maxfd = connfd;

			if (i > maxi)
				maxi = i;

			if (--nready <= 0)
				continue;
		}
		
		for (i = 0; i <= maxi; i++) 
		{
			if ((sockfd = client[i]) < 0)
				continue;

			if (FD_ISSET(sockfd, &rset)) 
			{
				if((n = Readn(connfd, recvline, MAXLINE))==0)
				{
					printf("[disconnected to %s:%d]\n", Inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf)), ntohs(cliaddr.sin_port));
					printf("[waiting for a talking client]\n");
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}
		
				else 
				{
					printf("<climsg> : %s\n", recvline);
					if (--nready <= 0)
						break;
				}
			}


			if (FD_ISSET(fileno(fp), &rset)) 
			{
				Fgets(sendline,MAXLINE,stdin);
				sendline[strlen(sendline) -1]='\0';

				if(strcmp(sendline,"/quit")==0)
				{
					stdineof = 1;
					Shutdown(sockfd, SHUT_WR);
					FD_CLR(fileno(fp), &rset);
					con_check=true;
					break;
				}


				Writen(sockfd, sendline, MAXLINE);

				if (--nready <= 0)
					break;
			}
		}
	}
}
