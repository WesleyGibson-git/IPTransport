#include "config.h" 
/**
 * Unix-style error
 */
static void unix_error(char * msg) 
{
    fprintf(stderr,"%s: %s\n",msg,strerror(errno));
    exit(0);
}

static void app_error(char *msg) /* Application error */
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}
/**
 * Open connection to server at <hostname,port> and return a socket descripter ready for reading and writing.
 * Return -1 and sets errno on Unix error.
 * Return -2 and sets h_errno on DNS(gethostbyname) error. 
 */
int open_clientfd(char *hostname, char *port) {
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections */
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }
  
    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) 
    {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
            break; /* Success */
        if (close(clientfd) < 0) 
        { /* Connect failed, try another */  
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        } 
    } 

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}

/*
 * Wrappers for reentent protocol-indepentent helpers
 */
int Open_clientfd(char * hostname,char * port)
{
    int rc;
    if((rc= open_clientfd(hostname,port))<0)
        app_error("Open_clientfd error");
    return rc;
}


ssize_t Read(int fd, void *buf, size_t count) 
{
    ssize_t rc;

    if ((rc = read(fd, buf, count)) < 0) 
    {
        unix_error("Read error");
    }
    return rc;
}

ssize_t Write(int fd, const void *buf, size_t count) 
{
    ssize_t rc;

    if ((rc = write(fd, buf, count)) < 0)
    {
        unix_error("Write error");
    }
    return rc;
}


char * Fgets(char * ptr,int n,FILE *stream)
{
    char * rptr;
    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
	    unix_error("Fgets error");
    return rptr;
}

void Fputs(const char *ptr, FILE *stream) 
{
    if (fputs(ptr, stream) == EOF)
	    unix_error("Fputs error");
}

void Close(int fd) 
{
    int rc;

    if ((rc = close(fd)) < 0)
	    unix_error("Close error");
}