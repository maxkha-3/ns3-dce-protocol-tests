#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>        /* for memset */
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define content "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut ac consequat est. Mauris ut tempor erat, sit amet suscipit felis. Nullam ut neque quis sapien vestibulum pellentesque. Maecenas mattis libero ullamcorper arcu malesuada feugiat. Fusce scelerisque a diam eget scelerisque. Cras pharetra mauris at libero maximus, vel consequat lacus vulputate. In non mauris id est tempor aliquet dapibus eget mi. Proin molestie auctor lacus, sit amet vehicula erat tincidunt nec. Fusce nec sapien tincidunt, pharetra sapien eget, interdum magna. Nam vehicula neque nisl, eget tempus elit commodo ac. Aliquam eget tempor ligula. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Proin eu vestibulum metus. Duis ornare nisl dui, vehicula fringilla tortor lobortis ac. In porta eget nunc vitae scelerisque. Cras nisi tellus, vehicula eget rutrum et, viverra eu massa. Fusce sagittis dolor enim. Nulla pretium arcu risus, eu gravida lacus vulputate et. Aenean laoreet felis aliquet mauris laoreet, et mollis enim mollis. Morbi in neque nec quam ornare tempor eu id risus. Aliquam venenatis, augue at feugiat consequat, libero ligula fermentum arcu, sed tempus lectus eros at sapien. Vivamus porttitor dolor sit amet eros faucibus vestibulum. Vestibulum augue dolor, dictum eget ipsum et, mollis ultricies enim. Nam convallis venenatis ex, vel viverra dolor congue et. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin vel bibendum elit. Fusce molestie vel tellus id porttitor. Ut aliquam, diam vitae rutrum varius, nunc massa pellentesque ligula, eu mattis urna metus at ipsum. Duis a laoreet ante, iaculis iaculis tortor. Vivamus volutpat justo lacinia est elementum tempus. Sed id justo lacus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Integer sodales placerat est tincidunt tincidunt. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus auctor nisl tellus, id hendrerit ligula consequat at. Duis nunc sapien, sagittis in lobortis eget, fringilla sit amet magna. Etiam rutrum, nisi rutrum ullamcorper sagittis, mi elit feugiat purus, at finibus erat arcu in sem. Maecenas vel sapien et quam fermentum vulputate id quis est. In eget leo porttitor turpis auctor efficitur. Cras nec semper urna, eu faucibus elit. Morbi nec enim diam. In ut placerat eros. Pellentesque porta id tellus in efficitur. Integer dignissim purus vel lorem pretium accumsan. Nullam in erat mauris. Maecenas urna arcu, ultrices et urna non, hendrerit sagittis tortor. Nam nec lacus neque. Sed risus nibh, blandit vel tincidunt sed, scelerisque eu est. Sed sodales rhoncus diam, vitae malesuada libero eleifend ac. Sed nulla sem, aliquam id urna eget, efficitur consectetur velit. Sed id erat et lorem fermentum laoreet. Sed mollis lectus at velit vestibulum condimentum. Suspendisse hendrerit diam eu velit hendrerit mattis. Cras a neque nec urna gravida porta sed vitae nibh. Phasellus in commodo magna. Maecenas libero sem, auctor ut tristique non, congue vitae nibh. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin porttitor mi non condimentum sagittis. In suscipit sem ut justo viverra volutpat. Curabitur porta tincidunt nulla posuere finibus. Phasellus turpis leo, hendrerit sed est a, eleifend luctus nisi. Quisque quis porttitor leo. Nunc hendrerit mattis aliquam. Donec ultricies sodales euismod. "

using namespace std;

// Sends lorem ipsum text to specified SCTP stream
void send_chars(int sock, int to_stream, int ttl, int flags, int num_chars) {
	int stat, char_i;
	int content_size = sizeof(content);
	int buffer_size = 1024;
	char buffer[buffer_size + 1];

	// Loop over characters to send
	for (char_i = 0; char_i < num_chars; char_i++) {
		int content_i = char_i % (content_size - 1);
		int buffer_i = char_i % buffer_size;

		buffer[buffer_i] = content[content_i];

		if ((char_i + 1 == num_chars) || (buffer_i + 1 == buffer_size)) {
			buffer[buffer_i + 1] = '\0';
			// The buffer is full, or we put in the required number of characters, so send
			stat = sctp_sendmsg(sock, buffer, (size_t) strlen(buffer), NULL, 0,
					0, flags, to_stream, ttl, 0);
		}
	}
}

// Sends a file to specified SCTP stream
void send_file(int sock, int to_stream, int ttl, int flags,
		const char* file_path) {
	int sctp_stat;
	int bytes;
	int buffer_size = 1024;
	FILE* filp = fopen(file_path, "rb");
	if (!filp) {
		printf("Error: could not open file %s\n", file_path);
		return;
	}

	char * buffer = new char[buffer_size];
	while ((bytes = fread(buffer, sizeof(char), buffer_size, filp)) > 0) {
		sctp_stat = sctp_sendmsg(sock, buffer, (size_t) bytes, NULL, 0, 0,
				flags, to_stream, ttl, 0);
	}
	fclose(filp);
}

void loop_send_chars(int sock, int num_streams, int ttl, int flags,
		int num_chars) {
	int stream_i;
	for (stream_i = 0; stream_i < num_streams; stream_i++) {
		send_chars(sock, stream_i, ttl, flags, (int)((float)num_chars)/num_streams);
	}
}

void loop_send_file(int sock, int num_streams, int ttl, int flags,
		const char* file_path) {
	int stream_i;
	for (stream_i = 0; stream_i < num_streams; stream_i++) {
		send_file(sock, stream_i, ttl, flags, file_path);
	}
}
int main(int argc, char **argv) {

	const int MAX_STREAMS = 512;
	int port = 3007;
	int sock_server, connect_sock, stat, slen, i;
	struct sctp_initmsg initmsg;
	struct sockaddr_in server_addr;
	struct sctp_event_subscribe s_events;
	struct sctp_status s_status;

	// Parse input arguments 
	unsigned int bytes_to_transfer = 0;
	unsigned int ttl = 0;
	unsigned int flags = 0;
	char* file_path_to_transfer = (char *)"\0";
	unsigned int num_streams = 1;
	char* receiver_ip = (char *)"\0";

	while ((i = getopt(argc, argv, "a:f:d:t:s:u")) != -1) {
		switch (i) {
		case 'a':
			receiver_ip = optarg;
			break;
		case 'd':
			bytes_to_transfer = atoi(optarg);
			break;
		case 'f':
			file_path_to_transfer = optarg;
			break;
		case 't':
			ttl = atoi(optarg);
			break;
		case 's':
			num_streams = atoi(optarg);
			break;
		case 'u':
			flags |= SCTP_UNORDERED;
			break;
		default:
			break;
		}
	}

	connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	memset(&initmsg, 0, sizeof(initmsg));

	// TODO: These should be sent as function parameters probably
	initmsg.sinit_num_ostreams = MAX_STREAMS;         // Number of Output Stream
	initmsg.sinit_max_instreams = MAX_STREAMS;      // Number of Input Stream
	initmsg.sinit_max_attempts = MAX_STREAMS;
	stat = setsockopt(connect_sock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg,
			sizeof(initmsg));
	if (stat < 0) {
		perror("setsockopt error\n");
		exit(-1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(receiver_ip);
	stat = connect(connect_sock, (struct sockaddr *) &server_addr,
			sizeof(server_addr));
	if (stat < 0) {
		perror("connect error\n");
		exit(-1);
	}

	memset(&s_events, 0, sizeof(s_events));
	s_events.sctp_data_io_event = 1;

	stat = setsockopt(connect_sock, SOL_SCTP, SCTP_EVENTS,
			(const void *) &s_events, 9);
	if (stat < 0) {
		perror("event error\n");
		exit(-1);
	}

	slen = sizeof(s_status);
	stat = getsockopt(connect_sock, SOL_SCTP, SCTP_STATUS, (void *) &s_status,
			(socklen_t *) &slen);

	printf("assoc id  = %d\n", s_status.sstat_assoc_id);
	printf("state     = %d\n", s_status.sstat_state);
	printf("instrms   = %d\n", s_status.sstat_instrms);
	printf("outstrms  = %d\n", s_status.sstat_outstrms);

	if ((file_path_to_transfer != NULL) && (file_path_to_transfer[0] != '\0')) {
		loop_send_file(connect_sock, num_streams, ttl, flags,
				file_path_to_transfer);
	}
	if (bytes_to_transfer > 0) {
		loop_send_chars(connect_sock, num_streams, ttl, flags,
				bytes_to_transfer);
	}

	/* Close our socket and exit */
	close(connect_sock);

	return 0;

}

