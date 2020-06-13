#include "tools.h"

static int blscan(blpNode blpAccount, const char *path);
/**
 * Unix-style error
 */
static void unix_error(char *msg)
{
    //fprintf(stderr,"%s: %s\n",msg,strerror(errno));
    syslog(LOG_ERR, "%s\n", msg);
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

void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags)
{
    int rc;
    if ((rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)) != 0)
    {
        syslog(LOG_ERR, "Getnameinfo error: %s\n", gai_strerror(rc));
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

    if ((rc = open(pathname, flags, mode)) < 0)
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

int writeList(const char *path, char *address)
{
    FILE *fd = NULL;
    if ((fd = fopen(path, "a")) == NULL)
    {
        syslog(LOG_ERR, "Open list file failed: %s\n", strerror(errno));
        unix_error("Open list file error");
    }
    fprintf(fd, "%s\n", address);
    fclose(fd);

    return 1;
}

void readConfig(char *port, char *blackListPath, char *whiteListPath, char *strsandwich)
{

    /* Read value from config file. Pay attention to the last line！*/
    struct ccl_t config;           // ccl variable
    const struct ccl_pair_t *iter; // ccl variable
    config.comment_char = '#';     // ccl comment char is '#'
    config.sep_char = '=';         // ccl Division symbol
    config.str_char = '"';         // ccl Specify string boundaries
    ccl_parse(&config, "transferAddress.conf");
    while ((iter = ccl_iterate(&config)) != 0)
    {
        if (!strcmp(iter->key, "PORT"))
        {
            strncpy(port, iter->value, 8); // get the value of port number from config file.
            continue;
        }
        if (!strcmp(iter->key, "WHITELISTPATH"))
        {
            strncpy(whiteListPath, iter->value, 256); // get the value of white list path from config file.
            continue;
        }
        if (!strcmp(iter->key, "BLACKLISTPATH"))
        {
            strncpy(blackListPath, iter->value, 256); // get the value of black list path from config file.
            continue;
        }
        if (!strcmp(iter->key, "SANDWICH"))
        {
            strncpy(strsandwich, iter->value, 112); // get the value of sandwich string from config file.
            continue;
        }
    }
    ccl_release(&config);
    /* Read config parameter failed */
    if (!(*port))
    {
        syslog(LOG_ERR, "Read port config failed: %s\n", strerror(errno));
        unix_error("Config error");
    }
    if (!(*whiteListPath))
    {
        syslog(LOG_ERR, "Read white list path config failed: %s\n", strerror(errno));
        unix_error("Config error");
    }
    if (!(*blackListPath))
    {
        syslog(LOG_ERR, "Read black list path config failed: %s\n", strerror(errno));
        unix_error("Config error");
    }
    if (!(*strsandwich))
    {
        syslog(LOG_ERR, "Read sandwich string config failed: %s\n", strerror(errno));
        unix_error("Config error");
    }
    /* End Read config parameter failed */
    /*End Read value from config file. */
}

int isValidIP(char *ip)
{
    if (INADDR_NONE == inet_addr(ip))
    {
        return -1;
    }
    return 1;
}
/* 
 *  Record the number of reco
 */
int addblacklist(char *hostname, blpNode blpAccount, const char *path)
{
    blpNode temp, cursor;
    if (blpAccount->next == NULL) // it's the first one
    {
        temp = malloc(sizeof(struct node));
        memset(temp, 0, sizeof(struct node));
        strncpy(blpAccount->hostname, hostname, IP4LONG); //copy hostname
        blpAccount->count++;                              // count add
        blpAccount->next = temp;
        return 0;
    }
    cursor = blpAccount;
    while (cursor->next != NULL && strcmp(hostname, cursor->hostname))
    {
        temp = cursor; // this temp point to the previous node
        cursor = cursor->next;
    }
    if (cursor->next) // there are existing node
    {
        cursor->count++;
        blscan(blpAccount, path);
        return 0;
    }
    else // don't find existing node so add new node
    {
        strncpy(cursor->hostname, hostname, IP4LONG); //copy hostname
        cursor->count++;
        cursor->pre = temp;
        temp = malloc(sizeof(struct node));
        memset(temp, 0, sizeof(struct node)); // count add
        cursor->next = temp;
        blscan(blpAccount, path);
        return 0;
    }
}
/* If some address ip recorded three times. Clean linked node and add it to black list.*/
static int blscan(blpNode blpAccount, const char *path)
{
    blpNode cursor;
    cursor = blpAccount;
    while (cursor->hostname != NULL)
    {
        if (cursor->count >= MAXWRONG)
        {
            //printf("warning,%s\n", cursor->hostname);
            writeList(path, cursor->hostname);
            syslog(LOG_INFO, "%s add to black list.\n", cursor->hostname);
            if (cursor->pre == NULL) // it's the first one
            {
                memset(cursor, 0, sizeof(struct node));
                return 0;
            }
            else
            {
                cursor->pre->next = cursor->next;
                cursor->next->pre = cursor->pre;
                free(cursor);
                return 0;
            }
        }
        cursor = cursor->next;
    }
    return 0;
}

int removeBLcache(char *hostname,blpNode blpAccount)
{
    blpNode cursor;
    cursor = blpAccount;
    while (cursor->next != NULL && strcmp(hostname, cursor->hostname))
    {
        cursor = cursor->next;
    }
    if (cursor->next)
    {
        cursor->pre->next=cursor->next;
        cursor->next->pre=cursor->pre;
        free(cursor);
        return 0;
    }
    return 0;
}