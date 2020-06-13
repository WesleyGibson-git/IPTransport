#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PUBLICKEY "publicKey1024.pem"
#define PRIVATEKEY "privateKey1024.pem"

/*
 *  Encryption by RSA public key 
 *  return -1: Public key path error
 *  return -2: PEM_read_RSA_PYBKEY error
 *  return -3: RSA_public_encrypt error 
 */
int RSA_encrypt_1024(char *from, char *to);

/*
 *  Decryption by RSA private key
 *  return -1: Private key path error
 *  retrun -2: PEM_read_RSAPrivateKey error
 *  return -3: RSA_private_decrypt error      
 */
int RSA_decrypt_1024(char *from, char *to);

/*
*   Debug ciphertext
*/
void printCipherText(char *target, int n);

char *strJuicer(char *target);
