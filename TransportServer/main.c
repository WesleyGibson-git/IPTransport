#include "tools.h"
#include "security.h"
#include <syslog.h>

char strsandwich[128];

int main(int argc, char **argv)
{
    openlog("TransferLog", LOG_CONS | LOG_PID, LOG_LOCAL2); //rsyslog
    pid_t pid;
    syslog(LOG_INFO, "Service Level1 start\n");
    if ((pid = Fork()) == 0)
    {
        Setsid(); // Create a new session with the calling process as its leader.
        //umask(0);
        int x;
        for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) // close all fd
        {
            close(x);
        }

        blpNode blpAccount;
        blpAccount = malloc(sizeof(struct node));
        memset(blpAccount, 0, sizeof(struct node));

        syslog(LOG_INFO, "Service Level2 start\n");
        char whiteListPath[256];
        char blackListPath[256];
        char port[8];

        readConfig(port, blackListPath, whiteListPath, strsandwich);

        int listenfd, connfd;
        socklen_t clientlen;
        struct sockaddr_storage clientaddr; /* Enough space for any address */
        char client_hostname[IP4LONG], client_port[IP4LONG];
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
            Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, IP4LONG, client_port, IP4LONG, NI_NUMERICHOST);
            syslog(LOG_INFO, "Connected to (%s, %s)\n", client_hostname, client_port); //log connected clients
            char *decryptMsg = calloc(128, sizeof(char *));                            // Use 1024 key
            if (Read(connfd, buf, 128) != 128)                                         // read data from client
            {
                // Add black list
                syslog(LOG_INFO, "The length of data is wrong. The data is %s\n", buf);
                //writeList(blackListPath, client_hostname);
                addblacklist(client_hostname, blpAccount, blackListPath);
                //syslog(LOG_INFO, "%s add to black list.\n", client_hostname);
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
                else if (error == -3) // RSA decrypt failed.
                {
                    syslog(LOG_INFO, "The encrypted str is wrong. The data is %s\n", buf);
                    //writeList(blackListPath, client_hostname);
                    addblacklist(client_hostname, blpAccount, blackListPath);
                    //syslog(LOG_INFO, "%s add to black list.\n", client_hostname);
                    Close(connfd);
                    continue;
                }
                else
                {
                    syslog(LOG_ERR, "RSA_decrypt_1024 unknown error.\n");
                    exit(0);
                }
            }
            if (strstr(decryptMsg, strsandwich) == NULL) // sandwich string is wrong.
            {
                syslog(LOG_INFO, "The SandWich str is wrong. The data is %s\n", buf);
                //writeList(blackListPath, client_hostname);
                addblacklist(client_hostname, blpAccount, blackListPath);
                //syslog(LOG_INFO, "%s add to black list.\n", client_hostname);
                Close(connfd);
                continue;
            }
            else
            {
                decryptMsg = strJuicer(decryptMsg);
                if (isValidIP(decryptMsg))
                {
                    writeList(whiteListPath, decryptMsg);
                    removeBLcache(client_hostname,blpAccount);
                    syslog(LOG_INFO, "%s add to white list.\n", decryptMsg);
                    Write(connfd, "ok", 2);
                    Close(connfd);
                    continue;
                }
                else // not a valid ip address.
                {
                    syslog(LOG_INFO, "Not a valid IP address. The data is %s\n", buf);
                    //writeList(blackListPath, client_hostname);
                    addblacklist(client_hostname, blpAccount, blackListPath);
                    //syslog(LOG_INFO, "%s add to black list.\n", client_hostname);
                    Close(connfd);
                    continue;
                }
            }
            Close(connfd);
        }
        closelog();
        return 0;
    }
    return 0;
}