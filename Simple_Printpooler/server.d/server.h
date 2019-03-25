#define QUEUE_SIZE 30
#define FILE_SIZE 20

void * handle_client(void * pool);
void * handle_printer(void * printer);
void add_job(char * filenaam);
char* take_job();

typedef struct {
  int connfd;
  struct sockaddr_in server;
} printer_conn_t;

typedef struct printer {
    char *hostname;
    int port;
    int busy;
    printer_conn_t *connection;
} printer_t;

printer_conn_t *printer_init(const char* server_addr, const int portno);
