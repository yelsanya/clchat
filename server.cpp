#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include <set>

#define PORT 5000

using namespace std;

void respond (int sock);
map<string, int> client[10];
set<string> room[10]; 

int main( int argc, char *argv[] ) {
  int sockfd, newsockfd, portno = PORT;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  clilen = sizeof(cli_addr);
  pid_t pid;
  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  // port reusable
  int tr = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* TODO : Now bind the host address using bind() call.*/
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		exit(1);
	}

  /* TODO : listen on socket you created */
	listen(sockfd, 5);


  printf("Server is running on port %d\n", portno);

  while (1) {
    /* TODO : accept connection */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    // TODO : implement processing. there are three ways - single threaded, multi threaded, thread pooled
    if (newsockfd < 0) {
	perror("Error on accept");
	exit(1);
    }
    pid = fork();
    if (pid < 0) 
    {
	perror("ERROR on fork");
	exit(1);
    }
    if (pid==0)
    {
	close(sockfd);
	respond(newsockfd);
	exit(0);
    }
    else close(newsockfd);
  }

  return 0;
}

void respond(int sock) {
    int n, check, datalen, rooms[10], wroom;
    char buffer[9999];
    string name, format;
    pid_t pid1;
    char *method;
    string ans;
    bzero(buffer,9999);
    n = read(sock,buffer,9999);
    if (n < 0) {
    printf("read() error\n");
    return;
    } else if (n == 0) {
    printf("Client disconnected unexpectedly\n");
    return;
    } else {
        method = strtok(buffer, " ,");
	wroom = method[0] - '0';
        method = strtok(NULL, " \n\0");
	format = method;
	bool dead, kl = 0;
        while(room[wroom].count(format) or format == "all") format = format + "-1";
	name = format;
	cout << name + " connected to room "<< wroom << endl;
	string test = "/test";
	char cwroom = wroom + '0';
	string wlcm = name + " joined room #" + cwroom;
	for(map<string, int>::iterator i = client[wroom].begin(); i != client[wroom].end(); i++)
	{
		write(i->second, wlcm.c_str(), wlcm.size());
	}
	write(sock, name.c_str(), name.size());
	client[wroom][name] = sock;
	room[wroom].insert(name);
	cout << room[wroom].size() << "\n";
	pid1 = fork();
	if (pid1 < 0) 
	{
		perror("ERROR on fork");
		exit(1);
	}
 	if( pid1 == 0) 
   	{
		time_t init, now;
		time(&init);
		dead = 0;
		while (true)
		{
			time(&now);
	    		if (((int)difftime(now, init))%3 == 0 and (int)now != (int)init) 
	    		{
				if(dead)
				{
					kl = 1;
					exit(0);
				}		
				write(sock, test.c_str(), test.size());
				dead = 1;
				cout << "test\n";
	 		}
		}
	}
        while (true) {
            bzero(buffer,9999);
	    n = read(sock, buffer, 9999);
	    if (n < 0)
	    {
		perror("ERROR reading from socket");
		exit(1);
	    }
	    method = strtok(buffer, " ,\0");
	    format = method;
		cout << format << endl;
	    if (format == "/quit" or kl) 
	    {
		char cwroom = wroom + '0';
		string bye = name + " disconnected from room #" + cwroom;
		client[wroom].erase(client[wroom].find(name));
		write(sock , format.c_str(), format.size()); 
		for(map<string, int>::iterator i = client[wroom].begin(); i != client[wroom].end(); i++)
		{
			write(i->second, bye.c_str(), bye.size());
		}
		break;
	    }
	    else if (format == "/join")
	    {
		char cwroom = wroom + '0';
		string bye = name + " disconnected from room #" + cwroom;
		client[wroom].erase(client[wroom].find(name));
		for(map<string, int>::iterator i = client[wroom].begin(); i != client[wroom].end(); i++)
		{
			write(i->second, bye.c_str(), bye.size());
		}
		method = strtok(NULL, " \0");
		wroom = method[0] - '0';
		wlcm = name + " joined room #" + method[0];
		for(map<string, int>::iterator i = client[wroom].begin(); i != client[wroom].end(); i++)
		{
			write(i->second, wlcm.c_str(), wlcm.size());
		}
		client[wroom][name] = sock;
		continue;
	    }
	    else if (format == "/list") 
	    {
		char cwroom = wroom + '0';
		string lst = "> This is list of users in room #" + cwroom + '\n';
		int count = 1;
		for(set<string>::iterator i = room[wroom].begin(); i != room[wroom].end(); i++)
		{
			string scount;
			if (count/10 != 0) 
			{
				scount[0] = count/10 + '0';  
				scount[1] = count%10 + '0';
				scount[2] = '\0';
			}
			else 
			{
				scount[0] = count + '0';
				scount[1] = '\0';
			}
			if(count != 1) lst = lst + '\n';
			lst = lst + scount + ". " + *i;
			count++;
		}
		write(sock, lst.c_str(), lst.size());
		continue;
	    }
	    else if (format == "/test") 
	    {
		dead = 0;
		cout << "test passed\n";
		continue;
	    }
	    string invus = "There is no such user : ";
	    string temp = method;
	    bool few = 0;
	    vector<int> adr;
            while(temp != ":")
	    {
		if(!client[wroom].count(temp)) 
		{
			if(temp == "all")
			{
				adr.clear();
				for(map<string, int>::iterator i = client[wroom].begin(); i != client[wroom].end(); i++)
				{
					if(i-> first != "name") adr.push_back(client[wroom][i->first]);
				}
				break;
			}
			if(few) invus = invus + ", ";
			invus = invus + temp;
			few = 1;
		}
		else adr.push_back(client[wroom][temp]);
		method = strtok(NULL, " ,");
		temp = method;
	    }
	    while(temp != ":")
	    {
		method = strtok(NULL, " ,");
		temp = method;
	    }
	    if (few) write(sock, invus.c_str(), invus.size());
	    method = strtok(NULL, "");
	    format = method;
	    for(int x = 0; x < adr.size(); x++) write(adr[x], format.c_str(), format.size());   
        }
	}
  cout << name + " disconnected from room " << wroom << endl;
  shutdown(sock, SHUT_RDWR);
  close(sock);
}
