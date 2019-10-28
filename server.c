#include "server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int resolve(const char *host, unsigned short port, struct addrinfo** addr) {
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE,
	};
	char port_buf[8];
	snprintf(port_buf, sizeof port_buf, "%u", port);
	return getaddrinfo(host, port_buf, &hints, addr);
}

int resolve_sa(const char *host, unsigned short port, union sockaddr_union *res) {
	struct addrinfo *ainfo = 0;
	int ret;
	SOCKADDR_UNION_AF(res) = AF_UNSPEC;
	if((ret = resolve(host, port, &ainfo))) return ret;
	memcpy(res, ainfo->ai_addr, ainfo->ai_addrlen);
	freeaddrinfo(ainfo);
	return 0;
}

int bindtoip(int fd, union sockaddr_union *bindaddr) {
	socklen_t sz = SOCKADDR_UNION_LENGTH(bindaddr);
	if(sz)
		return bind(fd, (struct sockaddr*) bindaddr, sz);
	return 0;
}

int server_waitclient(struct server *server, struct client* client) {
	socklen_t clen = sizeof client->addr;
	return ((client->fd = accept(server->fd, (void*)&client->addr, &clen)) == -1)*-1;
}

int server_setup(struct server *server, unsigned short port) {
	struct sockaddr_in addr;
    int listen_fd = -1;
    int opt = 1;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Try to bind to the listen port
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof (struct sockaddr_in)) == -1) {
        close(listen_fd);
        return -2;
    }

	if (listen(listen_fd, SOMAXCONN) < 0)
	{
		close(listen_fd);
		return -3;
	}
	server->fd = listen_fd;
	return 0;
}
