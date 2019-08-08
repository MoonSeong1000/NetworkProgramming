#include	"unp.h"

#include	"unp.h"

void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1, stdineof;
	fd_set rset;
	char buf[MAXLINE];
	char recvline[MAXLINE],sendline[MAXLINE];
	int n;
	 

	stdineof = 0;
	FD_ZERO(&rset);

	for (; ; ) 
	{
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(sockfd, &rset)) {
			if((n = Readn(sockfd, recvline, MAXLINE))==0)
			{
				printf("server quit\n");
				return;
			}
			printf("<srvmsg> : %s\n", recvline);
		}

		if (FD_ISSET(fileno(fp), &rset)) {
			Fgets(sendline,MAXLINE,stdin);
			sendline[strlen(sendline) -1]='\0';

			if (strcmp(sendline,"/quit")==0) 
			{
				stdineof = 1;
				Shutdown(sockfd, SHUT_WR);
				FD_CLR(fileno(fp), &rset);
				continue;
			}
			
			Writen(sockfd, sendline, MAXLINE);
		}
	}
}

int
main(int argc, char **argv)
{
	int					sockfd, n;
	struct sockaddr_in	servaddr;
	char buff[MAXLINE];

	if (argc != 3)
		err_quit("usage: a.out <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));	/* daytime server */
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));
	printf("[connected to %s:%d]\n", Inet_ntop(AF_INET, &servaddr.sin_addr, buff, sizeof(buff)), ntohs(servaddr.sin_port));
	str_cli(stdin, sockfd);

	exit(0);
}

