#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PUBLICKEY "publicKey1024.pem"
#define PRIVATEKEY "privateKey1024.pem"
#define SANDWICH "@@8462fd8cc8f6702b8d65622c3e5b0c6b992d107bb787509e6d1e8e0852fa4859ade0340977b13c85b7aa9cfb927cb10bf"
/*
*  The encrypt of RSA
*/
int RSA_encrypt_1024(char *from, char *to);

/*
*   The decrypt of RSA
*/
int RSA_decrypt_1024(char *from, char *to);

/*
*   Debug ciphertext
*/
void printCipherText(char *target, int n);

char *strJuicer(char *target);

