/*
 * prometheus_exporter.c
 *
 *  Created on: Aug 29, 2018
 *      Author: johan
 */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static pthread_t prometheus_tread;

static char prometheus_buf[1024*1024] = {0};
static int prometheus_buf_len = 0;
static int listen_fd;

static char header[1024] =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain;\r\n\r\n";


void * prometheus_exporter(void * param) {

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(9101);

retry_bind:
	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		printf("Cannot bind prometheus to port 9101\n");
		sleep(1);
		goto retry_bind;
	} else {
		printf("Prometheus exporter listening on port 9101\n");
	}

	listen(listen_fd, 100);

	while(1) {

		struct sockaddr_in client_addr;
		socklen_t client_addr_len;
		int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);

		int written = write(conn_fd, header, strlen(header));

		/* Dump queued data */
		written += write(conn_fd, prometheus_buf, prometheus_buf_len);
		printf("Prometheus sent %d bytes\n", written);

		prometheus_clear();

		close(conn_fd);

	}

	return NULL;

}

void prometheus_add(char * str) {
	strcpy(prometheus_buf + prometheus_buf_len, str);
	prometheus_buf_len += strlen(str);
}

void prometheus_clear(void) {
	prometheus_buf_len = 0;
}

void prometheus_init(void) {
	pthread_create(&prometheus_tread, NULL, &prometheus_exporter, NULL);
}

void prometheus_close(void) {
	close(listen_fd);
}


