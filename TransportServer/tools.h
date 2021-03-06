#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ccl/ccl.h"

#define IP4LONG 16 /* Max text line length */
#define LISTENQ 1024 /* Second argument to listen() */
#define RIO_BUFFSIZE 128
#define MAXWRONG 3

typedef struct sockaddr SA;

/* Record the number of errors */
struct node
{
    char hostname[16];
    int count;
    struct node *pre;
    struct node *next;
};
typedef struct node * blpNode;
/* When the wrong time greater than 3, add blacklist*/
int addblacklist(char *hostname,blpNode blpAccount,const char *path);
int removeBLcache(char *hostname,blpNode blpAccount);

/* Process control wrappers */
pid_t Fork(void);
pid_t Setsid(void);

/* Sockets interface wrappers */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/**
 * Open and return a listening socket on port.This function is reentrant and protocol-independent.
 * On error, returns:
 * -1 for getaddrinfo error
 * -2 for setsockopt error
 * -3 for open_listenfd close failed
 * -4 for no address worked
 */
int open_listenfd(char *port);

/* Wrappers for reentrant protocol-independent client/server helpers */
int Open_listenfd(char *);

/* Protocol independent wrappers */
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags);

/* Unix I/O wrappers */
int Open(const char *pathname, int flags, mode_t mode);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);
void Close(int fd);

/* White item in list file */
int writeList(const char *path, char *address);

/* Read parameter from the config file */
void readConfig(char *port, char *blackListPath, char *whiteListPath, char *strsandwich);

/* Verify IP address */
int isValidIP(char *ip);
