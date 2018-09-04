/*
** Peter Christakos - pechristakos@wpi.edu
** Project 1 - Client Server 'http_client.c'
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>


// get sock address for IPv4/IPv6
void *get_address(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) { // check if address is IPv4
		return &(((struct sockaddr_in*)sa)->sin_addr); // return IPv4 address
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr); // return IPv6 address
}


int main(int argc, char *argv[]) {
	struct timeval tv;
	time_t startTime, endTime, totalTIme;
	int RTT_Flag = 0; // 0 is no, 1 is yes
	int sockfd, numbytes;
	char buf[100];
	struct addrinfo info, *servinfo, *p; // servinfo and p will point to results
	int status; // error code variable
	char s[INET6_ADDRSTRLEN]; // char of IPv6 address
	const char * node;
	const char * service;

	printf("argv[1]: %s\n", argv[1]);
	printf("argv[2]: %s\n", argv[2]);
	printf("argv[3]: %s\n", argv[3]);
	

	// set RTT flag to true if 3 arguments and one is '-p'
	if (argc == 4) {
		if (strcmp(argv[1], "-p") == 0) {
			RTT_Flag = 1;
			node = argv[2];
			service = argv[3];
			printf("RTT Activated.\n");
			// print RTT
		} else { fprintf(stderr, "ERROR: Option not known\n"); exit(1); }	
	} else if (argc == 3) {
		node = argv[1];
		service = argv[2];
	} else {
		fprintf(stderr, "ERROR: Invalid amount of arguments\n"); exit(1);
	}

	memset(&info, 0, sizeof(info)); // make sure struct is empty
	info.ai_family = AF_UNSPEC; // don't care if IPv4 or 6
	info.ai_socktype = SOCK_STREAM; // TCP socket stream

	// if error code doesn't equal 0, throw an error
	// connection data saved to servinfo

	printf("Node: %s\n", node);
	printf("Service: %s\n", service);

	if ((status = getaddrinfo(node, service, &info, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status)); 
		return 1;
	}

	// servinfo now points to a linked list of 1 or more struct addrinfos


	// record time of day for RTT purposes
	gettimeofday(&tv, NULL);
	startTime = tv.tv_sec;

	// loop through all the results and connect to the first we can

	for(p = servinfo; p != NULL; p = p->ai_next) { 
		// socket function creates endpoint for communication. Returns
		// non-negative integer for success
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Client: Socket Error"); 
			continue;
		}
		// connect function returns 0 upon success
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("Client: Connection Error");
			close(sockfd);
			continue;
		}

		// record time of day for RTT purposes
		gettimeofday(&tv, NULL);
		endTime = tv.tv_sec;
		totalTIme = endTime - startTime;

		if (RTT_Flag == 1) {
			printf("Connected with Round Trip Time: %ld sec\n", totalTIme);
		}
		
		break;
	}

	// throw null error for p if connection failed
	if (p == NULL) {
		fprintf(stderr, "Client: Failed to Connect\n");
		return 2;
	}

	// convert IPv4 or IPv6 from binary to text
	inet_ntop(p->ai_family, get_address((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("Client: Connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	printf("before recv function\n");

	if ((numbytes = recv(sockfd, buf, 99, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	printf("after recv function\n");

	buf[numbytes] = '\0';
	printf("Client: Received! '%s'\n",buf);
	close(sockfd); // close socket connection

	return 0;
}


