#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
  int connfd;
  struct sockaddr_in server;
} client_conn_t;

client_conn_t *client_init(const char* server_addr, const int portno);

void client_send_task(client_conn_t *client_conn);