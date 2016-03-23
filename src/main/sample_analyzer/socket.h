#ifndef _SOCKET_CS
#define _SOCKET_CS


#include <string>
#if defined WIN32 || defined WIN64
  #include <winsock2.h>
  #include "iso646.h"
  #define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
  #define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)
  #define read_from_socket(sck,bf,sz) recv(sck,bf,sz,NULL)
  #define write_to_socket(sck,str,sz) send(sck,str,sz,NULL)
  #define close_socket(sck) closesocket(sck)
  #define startup_socket() WSADATA wsaData; \
                    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData); \
                    if (iResult != NO_ERROR) \
                       error("Error at WSAStartup()\n", iResult);
  #define cleanup_socket() WSACleanup()
  #define socklen_t int
#else
  #include <string.h>
  #include <cstdlib>
  #include <cstdio>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #define read_from_socket(sck,bf,sz) read(sck,bf,sz)
  #define write_to_socket(sck,str,sz) write(sck,str,sz)
  #define close_socket(sck) close(sck);
  #define startup_socket()
  #define cleanup_socket()
  #define SOCKET int
#endif

#define SOCK_QUEUE_SZ 5
#define BUFF_SZ 2048

class socket_CS {
  private:
  SOCKET sock, sock2;
  void error(const std::string &,int) const;

  public:
    socket_CS(int port, int qsize=SOCK_QUEUE_SZ);
    socket_CS(const std::string&, int);

    ~socket_CS();

    void wait_client();
    int read_message(std::string&);
    void write_message(const std::string &);
    void close_connection();
    void set_child();
    void set_parent();
};


void socket_CS::error(const std::string &msg, int code) const {
  perror(msg.c_str());
  exit(code);
}


socket_CS::socket_CS(int port, int queue_size) {
  struct sockaddr_in server;
  startup_socket();  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) error("ERROR opening socket", sock);
  int len=sizeof(server);
  bzero((char *) &server, len);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  
  int n=bind(sock, (struct sockaddr *) &server,len);
  if (n < 0) error("ERROR on binding",n);

  listen(sock,queue_size);
}



socket_CS::socket_CS(const std::string &host, int port) {

  struct sockaddr_in server;
  startup_socket();  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) error("ERROR opening socket",sock);

  struct hostent *hp = gethostbyname(host.c_str());
  if (hp == NULL) error("Unknown host",0);

  bzero((char *) &server, sizeof(server));
  server.sin_family = AF_INET;
  bcopy((char *)hp->h_addr,(char *)&server.sin_addr.s_addr,hp->h_length);
  server.sin_port = htons(port);

  int n=connect(sock,(struct sockaddr *) &server,sizeof(server));
  if (n < 0) error("ERROR connecting",n);

  sock2=sock;
}


socket_CS::~socket_CS() {
  cleanup_socket();
}

void socket_CS::wait_client() {  
  struct sockaddr_in client;
  socklen_t len = sizeof(client);
  sock2 = accept(sock,(struct sockaddr *) &client, &len);
  if (sock2 < 0) error("ERROR on accept",sock2);
}

void socket_CS::set_child() {  
  int n;
  n = close_socket(sock);
  if (n < 0) error("ERROR closing socket",n);
}

void socket_CS::set_parent() {  
  int n;
  n = close_socket(sock2);
  if (n < 0) error("ERROR closing socket",n);
}


int socket_CS::read_message(std::string &s) {
    
  int n,nt;
  char buffer[BUFF_SZ+1];

  s.clear();
  n = read_from_socket(sock2,buffer,BUFF_SZ);
  if (n < 0) error("ERROR reading from socket",n);
  buffer[n]=0;
  s = s + std::string(buffer);
  nt = n;
  while (n>0 and buffer[n-1]!=0) {
    n = read_from_socket(sock2,buffer,BUFF_SZ);
    if (n < 0) error("ERROR reading from socket",n);
    buffer[n]=0;
    s=s + std::string(buffer);
    nt += n;
  }

  return nt;
}

void socket_CS::write_message(const std::string &s) {
  int n;
  n = write_to_socket(sock2,s.c_str(),s.length()+1); 
  if (n < 0) error("ERROR writing to socket",n);
}


void socket_CS::close_connection() {
  int n;
  n=close_socket(sock2);
  if (n < 0) error("ERROR closing socket",n);
}


#endif
