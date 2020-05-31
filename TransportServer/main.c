#include "tools.h"
#include "security.h"
#include <syslog.h>
#include "ccl/ccl.h"

static void signal_kill();
extern char *strsandwich;
int main(int argc, char **argv)
{
    openlog("TransferLog", LOG_CONS | LOG_PID, LOG_LOCAL2); //rsyslog
    pid_t pid;
    syslog(LOG_INFO, "Service Level1 start\n");
    if ((pid = Fork()) == 0)
    {
        Setsid();   // Create a new session with the calling process as its leader.
        //umask(0);   
        int x;
        for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--)    // close all fd
        {
            close(x);
        }
        syslog(LOG_INFO, "Service Level2 start\n");
        char port[6];
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
                //strcpy(port,iter->value);   // get the value of port number from config file.
                strncpy(port, iter->value, 6); // get the value of port number from config file.
        }
        ccl_release(&config);
        /*End Read value from config file. */

        int listenfd, connfd;
        socklen_t clientlen;
        struct sockaddr_storage clientaddr; /* Enough space for any address */
        char client_hostname[MAXLINE], client_port[MAXLINE];
        size_t n;
        char buf[128];
        int error;

        if ((listenfd = Open_listenfd(port)) < 0)
        {
            if (listenfd == -2)
                syslog(LOG_ERR, "Getaddrinfo error. '%s'\n", argv[0]);
            else
                syslog(LOG_ERR, "Open_listenfd close failed. '%s'\n", argv[0]);
        }
        while (1)
        {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, NI_NUMERICHOST);
            syslog(LOG_INFO, "Connected to (%s, %s)\n", client_hostname, client_port); //log connected clients
            char *decryptMsg = calloc(128, sizeof(char *));                            // Use 1024 key
            if (Read(connfd, buf, 128) != 128)                                         // read data from client
            {
                // Add black list
                syslog(LOG_ERR, "Received data length error. The data is %s\n", buf);
                printf("black list test.\n");
                Close(connfd);
                continue;
            }
            if ((error = RSA_decrypt_1024(buf, decryptMsg)) < 0)
            {
                if (error == -1)
                {
                    syslog(LOG_ERR, "Private key path error.\n");
                    exit(0);
                }
                else if (error == -2)
                {
                    syslog(LOG_ERR, "PEM_read_RSAPrivateKey error.\n");
                    exit(0);
                }
                else if (error == -3)
                {
                    syslog(LOG_ERR, "RSA_private_decrypt error.\n");
                    // Add black list
                    Close(connfd);
                    continue;
                }
                else
                {
                    syslog(LOG_ERR, "RSA_decrypt_1024 unknown error.\n");
                    exit(0);
                }
            }
            if (strstr(decryptMsg, strsandwich) == NULL)
            {
                printf("black list test.\n");
            }
            else
            {
                printf("Add list test\n");
                decryptMsg = strJuicer(decryptMsg);
                printf("%s\n", decryptMsg);
                Write(connfd, "ok", 2);
                Close(connfd);
                continue;
            }
            Close(connfd);
        }
        closelog();
        syslog(LOG_INFO, "Service stoped.\n");
        return 0;
    }
    return 0;
}