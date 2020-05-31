#include "tools.h"

/**
 * Unix-style error
 */
static void unix_error(char *msg)
{
    //fprintf(stderr,"%s: %s\n",msg,strerror(errno));
    syslog(LOG_ERR, "%s\n",msg);
    exit(0);
}
pid_t Fork(void) 
{
    pid_t pid;

    if ((pid = fork()) < 0)
	    unix_error("Fork error");
    return pid;
}

pid_t Setsid(void)
{
    pid_t pid;
    if (setsid() < 0)
	    unix_error("Setsid error");
    return pid;
}

/**
 * Open and return a listening socket on port.This function is reentrant and protocol-independent.
 * On error, returns:
 * -1 for getaddrinfo error
 * -2 for setsockopt error
 * -3 for open_listenfd close failed
 * -4 for no address worked
 */
int open_listened(char *port)
{
    struct addrinfo hints, *lisp, *p;
    int listenfd, rc, optval = 1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;                   /* Allow IPv4 */
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;
    if ((rc = getaddrinfo(NULL, port, &hints, &lisp)) != 0)
    {
        //fprintf(stderr,"getaddrinfo failed (port %s): %s\n",port,gai_strerror(rc));
        syslog(LOG_ERR, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -1;
    }

    /* Walk the list for one that we can bind to */
    for (p = lisp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) == -1)
        {
            syslog(LOG_ERR, "Setsockopt failed: %s\n", strerror(errno));
            return -2;
        }
        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;               /* Success */
        if (close(listenfd) < 0) /* Bind failed, try the next */
        {
            //fprintf(stderr,"open_listenfd close failed: %s\n", strerror(errno));
            syslog(LOG_ERR, "Listen fd close failed when try to close iterate addresses: %s\n", strerror(errno));
            return -3;
        }
    }

    /* Clean up */
    freeaddrinfo(lisp);
    if (!p)
    {
        syslog(LOG_ERR, "No address worked, Please check internet connected.\n");
        return -4;
    } /* No address worded */

    /* Make it listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
    {
        if (close(listenfd) < 0) /* Bind failed, try the next */
        {
            //fprintf(stderr,"open_listenfd close failed: %s\n", strerror(errno));
            syslog(LOG_ERR, "Listenfd close failed after correct listen open failed : %s\n", strerror(errno));
            return -3;
        }
        syslog(LOG_ERR, "Listen open failed: %s\n", strerror(errno));
        return -5;
    }
    return listenfd;
}

int Open_listenfd(char *port)
{
    int rc;

    if ((rc = open_listened(port)) < 0)
        unix_error("Open_listenfd error");
    return rc;
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    int rc;
    if ((rc = accept(s, addr, addrlen)) < 0)
    {
        syslog(LOG_ERR, "Accept failed: %s\n", strerror(errno));
        unix_error("Accept error.");
    }
    return rc;
}

void Getnameinfo(const struct sockaddr *sa,socklen_t salen,char *host,size_t hostlen,char *serv,size_t servlen,int flags)
{
    int rc;
    if((rc=getnameinfo(sa,salen,host,hostlen,serv,servlen,flags))!=0)
    {
        syslog(LOG_ERR, "Getnameinfo error: %s\n",gai_strerror(rc));
        exit(0);
    }
}


void Close(int fd)
{
    int rc;

    if ((rc = close(fd)) < 0)
    {
        syslog(LOG_ERR, "Close failed: %s\n", strerror(errno));
        unix_error("Close error");
    }
}
int Open(const char *pathname, int flags, mode_t mode) 
{
    int rc;

    if ((rc = open(pathname, flags, mode))  < 0)
    {
        syslog(LOG_ERR, "Open failed: %s\n", strerror(errno));
        unix_error("Open error");
    }
    return rc;
}

ssize_t Read(int fd, void *buf, size_t count) 
{
    ssize_t rc;

    if ((rc = read(fd, buf, count)) < 0) 
    {
        syslog(LOG_ERR, "Read failed: %s\n", strerror(errno));
        unix_error("Read error");
    }
    return rc;
}

ssize_t Write(int fd, const void *buf, size_t count) 
{
    ssize_t rc;

    if ((rc = write(fd, buf, count)) < 0)
    {
        syslog(LOG_ERR, "Write failed: %s\n", strerror(errno));
        unix_error("Write error");
    }
    return rc;
}

