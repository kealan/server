#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT "8000"
#define BACKLOG 10
#define MAXPAYLOADSIZE 500
#define MAXDATASIZE 1000
#define MAXVALUESIZE 100
#define MAXMESSAGESIZE 100

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handler(int sock);

int parseData(char* buffer, char* dst, char* message, char* start);

int main()
{
    // listen on sock_fd, new connection on new_fd
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    // connector's address information
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: sss %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    // main accept() loop
    while(1)
    {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        // this is the child process
        if (!fork())
        {
            // child doesn't need the listener
            close(sockfd);
            handler(new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}


int parseData(char* buffer, char* dst, char* message, char* start)
{
    char* end = "\"";
    char* p1 = strstr(buffer,start);
    if (p1 == NULL)
    {
        sprintf(message,"Input value not found %s", start);
        return 1;
    }

    p1 = p1 + strlen(start) + 4;
    char* p2 = strstr(p1,end);
    if (p2 == NULL)
    {
        sprintf(message,"Malformed input");
        return 2;
    }

    int n = (int) (p2 - p1);
    if (n<MAXVALUESIZE)
    {
        strncpy(dst,p1,n);
        dst[n] = '\0';
    }
    else
    {
        sprintf(message,"Max value size exceded size:%d MAXVALUESIZE: %d\n", n, MAXVALUESIZE);
        return 3;
    }
#ifdef DEBUG
    printf("p1 %d \n", ((int) (p1 - buffer)));
    printf("p2 %d \n", ((int) (p2 - buffer)));
    printf("p2 - p1 %d \n", ((int) (p2 - p1)));
    printf("p1 string %s \n", p1);
    printf("p2 string %s \n", p2);
    printf("dst %s \n", dst);
#endif
    return 0;
}


void handler(int sock)
{
    int n1 = 0;
    int n2 = 0;
    int len = 0;
    char* server ="test";
    char* matchData;
    char* matchStatus;
    int dataLen=0;
    int serverError=0;
    int clientError=0;

    // Get time
    time_t now;
    time(&now);
    char* ctime_str = ctime(&now);

    // Chomp line
    int l = strlen(ctime_str) - 1;
    ctime_str[l] = '\0';

    // Max Value1 size
    char value1[MAXVALUESIZE];
    bzero(value1,MAXVALUESIZE);

    // Max Value2 size
    char value2[MAXVALUESIZE];
    bzero(value2,MAXVALUESIZE);

    // Max message size
    char message[MAXMESSAGESIZE];
    bzero(message,MAXMESSAGESIZE);

    // Max playload data to send
    char data[MAXPAYLOADSIZE];
    bzero(data,MAXPAYLOADSIZE);

    // Max data to transfer
    char buf[MAXDATASIZE];
    bzero(buf,MAXDATASIZE);
    if( (n1 = recv(sock, buf, MAXDATASIZE, 0)) == -1)
    {
        perror("recv failed");
        serverError=1;
    }

#ifdef DEBUG
    printf("num bytes received %d\n", n1);
    printf("server received %s\n", buf);
#endif

    // Strings to search for in received data
    matchData = strstr (buf,"POST /data HTTP");
    matchStatus = strstr (buf,"POST /status HTTP");

    // Search buffer for data
    if (matchData)
    {
        char* start = "user";
        int rtn = parseData(buf, value1, message, start);
        if (rtn)
        {
            printf("Error code %d message %s\n", rtn, message);
            clientError=1;
        }

        char* start2 = "email";
        rtn = parseData(buf, value2, message, start2);
        if (rtn)
        {
            printf("Error code %d message %s\n", rtn, message);
            clientError=1;
        }
#ifdef DEBUG
        printf("value1 %s \n", value1);
        printf("value2 %s \n", value2);
#endif
    }

    bzero(buf,MAXDATASIZE);

    if (serverError)
    {
        dataLen = snprintf(data,MAXPAYLOADSIZE,"{\"message\": \"Internal Server Error\",\"service\": \"%s\", \"version\": \"%s\"}\n",SERVICE, VERSION);
        printf("%d %s\n", dataLen, data);
        /* Header + a blank line + data*/
        len = snprintf(buf,MAXDATASIZE,"HTTP/1.1 500 Internal Server Error\nContent-Type: application/json\nContent-Length: %d\nServer: %s\nDate: %s\n\n%s", dataLen, server, ctime_str, data);
        serverError=0;
    }
    else if (clientError)
    {
        dataLen = snprintf(data,MAXPAYLOADSIZE,"{\"message\": \"%s\",\"service\": \"%s\", \"version\": \"%s\"}\n",message, SERVICE, VERSION);
        printf("%d %s\n", dataLen, data);
        /* Header + a blank line + data*/
        len = snprintf(buf,MAXDATASIZE,"HTTP/1.1 400 Bad Request\nContent-Type: application/json\nContent-Length: %d\nServer: %s\nDate: %s\n\n%s", dataLen, server, ctime_str, data);
        clientError=0;
    }
    else if (matchStatus)
    {
        dataLen = snprintf(data,MAXPAYLOADSIZE,"{\"message\": \"OK\",\"service\": \"%s\", \"version\": \"%s\"}\n",SERVICE, VERSION);
        printf("%d %s\n", dataLen, data);
        /* Header + a blank line + data*/
        len = snprintf(buf,MAXDATASIZE,"HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: %d\nServer: %s\nDate: %s\n\n%s", dataLen, server, ctime_str, data);
    }
    else if (matchData)
    {
        dataLen = snprintf(data,MAXPAYLOADSIZE,"{\"message\": \"OK\",\"user\": \"%s\",\"email\": \"%s\",\"service\": \"%s\", \"version\": \"%s\"}\n",value1,value2,SERVICE, VERSION);
        printf("%d %s\n", dataLen, data);
        /* Header + a blank line + data*/
        len = snprintf(buf,MAXDATASIZE,"HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: %d\nServer: %s\nDate: %s\n\n%s", dataLen, server, ctime_str, data);
    }
    else
    {
        dataLen = snprintf(data,MAXPAYLOADSIZE,"{\"message\": \"Bad Request\",\"service\": \"%s\", \"version\": \"%s\"}\n",SERVICE, VERSION);
        printf("%d %s\n", dataLen, data);
        /* Header + a blank line + data*/
        len = snprintf(buf,MAXDATASIZE,"HTTP/1.1 400 Bad Request\nContent-Type: application/json\nContent-Length: %d\nServer: %s\nDate: %s\n\n%s", dataLen, server, ctime_str, data);
    }


    // Send data
    if ((n2 = send(sock, buf, len, 0)) == -1)
    {
        perror("send");
        serverError=1;
    }

    if (serverError)
    {
        dataLen = snprintf(data,MAXPAYLOADSIZE,"{\"message\": \"Internal Server Error\",\"service\": \"%s\", \"version\": \"%s\"}\n",SERVICE, VERSION);
        printf("%d %s\n", dataLen, data);
        /* Header + a blank line + data*/
        len = snprintf(buf,MAXDATASIZE,"HTTP/1.1 500 Internal Server Error\nContent-Type: application/json\nContent-Length: %d\nServer: %s\nDate: %s\n\n%s", dataLen, server, ctime_str, data);
        serverError=0;
    }

    if ((n2 = send(sock, buf, len, 0)) == -1)
    {
        perror("send");
    }
    printf("num bytes sent %d\n", n2);
}
