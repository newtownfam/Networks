/*
** Peter Christakos - pechristakos@wpi.edu
** Project 1
** Part 1 - Client 'http_client.c'
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
#include<fcntl.h> //fcntl


#define MAXDATASIZE 100


// get sock address for IPv4/IPv6
void *get_address(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) { // check if address is IPv4
		return &(((struct sockaddr_in*)sa)->sin_addr); // return IPv4 address
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr); // return IPv6 address
}

/*
    Receive data in multiple chunks by checking a non-blocking socket
    Timeout in seconds
*/


int main(int argc, char *argv[]) {
	struct timeval tv;
	time_t startTime, endTime, totalTime;
	int RTT_Flag = 0; // 0 is no, 1 is yes
	int wait_counter = 0; // for looping through buffer and seeing EOF
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo info, *servinfo, *p; // servinfo and p will point to results
	int status; // error code variable
	char s[INET6_ADDRSTRLEN]; // char of IPv6 address
	char * node; // values to assign to inputs
	const char * port; // value to assign to input
	char * nodeAddress;
	char * pch;
	char * nodePath = malloc(100*sizeof(char));
	char message[100];
	int len;
	char one[50]; // two arrays for copying/tokeinizing purposes 
	char two[50];

	// set RTT flag to true if 3 arguments and one is '-p'
	if (argc == 4) {
		if (strcmp(argv[1], "-p") == 0) {
			RTT_Flag = 1;
			node = argv[2];
			port = argv[3];
			printf("\n");
			printf("RTT Activated.\n");
			// print RTT
		} else { fprintf(stderr, "ERROR: Option not known\n"); exit(1); }
	} else if (argc == 3) {
		node = argv[1];
		port = argv[2];
	} else {
		fprintf(stderr, "ERROR: Invalid amount of arguments\n"); exit(1);
	}


	// Tokenize the input 
	
	pch = strtok(node, "/");
	nodeAddress = pch;
	int count = 0;

	while(pch != NULL) {
	
		if (count != 0) {
			strcpy(one, "/");
			strcpy(two, pch);
			strcat(nodePath , strcat(one, two));
			//printf("%s\n", nodePath);
		}
		count++;
		pch = strtok(NULL, "/");
	}

	// add a '/' to the end of the path
	strcpy(one, "/");
	strcat(nodePath, one);

	printf("NodeAddress: %s\n", nodeAddress);
	printf("NodePath = %s\n", nodePath);

	
	// set up GET message
	if (strcmp(nodePath, "") == 0) {
		sprintf(message, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", node);
		len = strlen(message);
	} else {
		sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", nodePath, nodeAddress);
		len = strlen(message);
	}

	memset(&info, 0, sizeof(info)); // make sure struct is empty
	info.ai_family = AF_UNSPEC; // don't care if IPv4 or 6
	info.ai_socktype = SOCK_STREAM; // TCP socket stream

	// if error code doesn't equal 0, throw    an error
	// connection data saved to servinfo

	printf("\n");
	printf("Node: %s\n", node);
	printf("Port: %s\n", port);
	printf("\n");

	if ((status = getaddrinfo(node, port, &info, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}
	
	// servinfo now points to a linked list of 1 or more struct addrinfos

	// record time of day for RTT purposes
	gettimeofday(&tv, NULL);
	startTime = tv.tv_sec * 1000 + (tv.tv_usec) / 1000;

	// loop through all the results and connect to the first we can

	for(p = servinfo; p != NULL; p = p->ai_next) {

		// socket function creates endpoint for communication. Returns
		// non-negative integer for success
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Client: Socket Error");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Client: Connection Error");
            break;
        }

		// record time of day for RTT purposes
		gettimeofday(&tv, NULL);
		endTime = tv.tv_sec * 1000 + (tv.tv_usec) / 1000;

		totalTime = (endTime - startTime);

		if (RTT_Flag == 1) {
			printf("Connected with Round Trip Time: %ld milliseconds\n\n", totalTime);
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

    if(len != (send(sockfd, message, strlen(message), 0))) {
    perror("client: send");
    exit(1);
    }

    // loop through and print everything in the buffer
    
    fcntl(sockfd, F_SETFL, O_NONBLOCK); // sets socket to non blocking - prevents hanging
   
    while(1) {
    	memset(buf ,0 , MAXDATASIZE);  //clear the variable
        if((numbytes =  recv(sockfd , buf , MAXDATASIZE-1 , 0) ) < 0) {
        	// wait and try again, if greater than 1 then EOF
        	sleep(1);
        	wait_counter ++;
        	if (wait_counter > 1) {
        		break;
        	}
        } else {
            printf("%s" , buf);
            wait_counter = 0;
        }
    }

 
	printf("\nClient: Complete!\n\n");
	close(sockfd); // close socket connection

	return 0;
}
