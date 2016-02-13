#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "SenderContent.h"

#define SERVER_PORT 3007

using namespace std;

int main(int argc, char *argv[]) {
	struct hostent *host;
	struct sockaddr_in address;
	int master_socket, i, status;

	// Last buffer byte holds null terminator to give 1024 data size in packet.
	int buffer_size = 1025;
	char buffer[buffer_size];

	unsigned int bytes_to_transfer = 0;
	char* receiver_ip = (char *) "\0";

	// Parse input arguments 
	while ((i = getopt(argc, argv, "a:d:t:s:u")) != -1) {
		switch (i) {
		case 'a':
			receiver_ip = optarg;
			break;
		case 'd':
			bytes_to_transfer = atoi(optarg);
			break;
		default:
			break;
		}
	}
	// Create a socket and connect to server IP
	master_socket = socket(PF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_port = htons(SERVER_PORT);
	host = gethostbyname(receiver_ip);
	memcpy(&address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	if (connect(master_socket, (const struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("TCP: Connect failed\n");
		exit(EXIT_FAILURE);
	}
	// Send the specified amount of characters
	SenderContent content(bytes_to_transfer);
	while (content.fill(buffer, buffer_size)) {
		send(master_socket, buffer, (size_t) strlen(buffer), 0);
	}
	close(master_socket);
	return 0;
}