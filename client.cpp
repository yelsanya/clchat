#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <iostream>
#include <string>

using namespace std;
int main(int argc, char *argv[]) {
   if (argc < 4) 
   {
	cout << "Usage: ./client [address] [room number] [name]\n";
	return 0;
   } 
   string addr = argv[1], rm = argv[2], nm = argv[3];
   pid_t pid;
   if (rm[1] != '\0' or rm[0] < '0' or rm[0] > '9')
   {
	cout << "Acceptable rooms are: 0 - 9\n";
	return 0;
   }
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   char buffer[9999];
   portno = 5000;
   
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ) { perror("ERROR connecting"); exit(1);}
   string format = rm + " " + nm;
   string wr, rd, temp = "";
   write(sockfd, format.c_str(), format.size());
   bzero(buffer,9999);
   read(sockfd, buffer, 9999);
   string name = buffer;
   cout << "Hello " + name + "! This is room " + rm + "\n";
   pid = fork();
   if (pid < 0) 
   {
	perror("ERROR on fork");
	exit(1);
   }
   if( pid == 0) 
   {
	while(1)
	{
		wr = "";
		while (temp != ":") 
		{
			cin >> temp;
			if(wr == "")
			{
				if(temp == "/list")
				{
					n = write(sockfd, temp.c_str(), temp.size());
				     	if ( n < 0) 
					{
						perror("ERROR writing to socket");
						exit(1);
					}
					continue;
				}
				else if(temp == "/quit")
				{
					n = write(sockfd, temp.c_str(), temp.size());
				     	if ( n < 0) 
					{
						perror("ERROR writing to socket");
						exit(1);
					}
					return 0;
				}
				else if(temp == "/join")
				{
					int x;
					cin >> x;
					if (x > 9 or x < 0) 
					{
						cout << "SYSTEM MESSAGE: Acceptable rooms are: 0 - 9\n";
						continue;
					}
					char y = x + '0';
					temp = temp + " " + y;
					n = write(sockfd, temp.c_str(), temp.size());
				     	if ( n < 0) 
					{
						perror("ERROR writing to socket");
						exit(1);
					}
					cout << "Hello " + name + "! This is room " + y + "\n";
					continue;
				}
			} 
			wr = wr + temp;
		}
		getline(cin, temp);
		wr = wr + temp;
		n = write(sockfd, wr.c_str(), wr.size());
	     	if ( n < 0) 
		{
			perror("ERROR writing to socket");
			exit(1);
		}
		
	}
   }
   else
   {	
	   while (true) {
		rd = "";
		bzero(buffer,9999);
		n = read(sockfd, buffer, 9999);
	     	if ( n < 0) 
		{
			perror("ERROR reading from socket");
			exit(1);
		}
		rd = buffer;
		//if (rd == "/test") cout << rd << endl;
		//break;
		if (rd == "/test") { write(sockfd, rd.c_str(), rd.size());  continue;}
		if (rd == "/quit") break;
		printf("%s\n", buffer);
	   }
   }
   cout << "Good bye " << name << endl;
   return 0;
}
