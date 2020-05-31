#include "config.h"
#include "security.h"

int isValidIP(char *ip);
void earseLine(char *);

int main(int argc, char **argv)
{
    
    int clientfd;
    char *host, *port, buf[MAXLINE];

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host><port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    printf("Please enter the address you want to operate: ");
    Fgets(buf, MAXLINE, stdin);
    earseLine(buf);
/*
    printf("Are you sure?(y/n): ");
    char answer = fgetc(stdin);
    if (answer != 'y')
        exit(0);t
*/
    if (!isValidIP(buf))
    {
        printf("The ip address is invalid!");
        exit(0);
    }
    char *encryptMsg = calloc(128,sizeof(char *));  //Use 1024 key 
    strcat(buf,SANDWICH);
    RSA_encrypt_1024(buf,encryptMsg);  

    clientfd = Open_clientfd(host, port);
    Write(clientfd, encryptMsg, 128);
    memset(buf, 0, MAXLINE);
    Read(clientfd, buf, MAXLINE); // Read from Server
    //Fputs(buf, stdout);
    if (strcasecmp(buf, "ok"))
    {
        printf("Failed");
    }
    else
    {
        printf("OK");
    }

    Close(clientfd);
    return 0;
}


int isValidIP(char *ip)
{
    if (INADDR_NONE == inet_addr(ip))
    {
        return -1;
    }

    return 1;
}

void earseLine(char *target)
{
    char *cusor = target;
    int count = 1;
    while ((*cusor) != '\n' && count < MAXLINE)
    {
        cusor++;
        count++;
    }
    *cusor = '\0';
}

