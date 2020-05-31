#include "security.h"

const char * strsandwich ="@@8462fd8cc8f6702b8d65622c3e5b0c6b992d107bb787509e6d1e8e0852fa4859ade0340977b13c85b7aa9cfb927cb10bf";
/*
 *  Encryption by RSA public key 
 *  return -1: Public key path error
 *  return -2: PEM_read_RSA_PYBKEY error
 *  return -3: RSA_public_encrypt error
 *  return 0 : Success  
 */
int RSA_encrypt_1024(char *from, char *to)
{
    FILE *fp = NULL;
    RSA *publicRsa = NULL;

    if ((fp = fopen(PUBLICKEY, "r")) == NULL)
    {
        //printf("public key path error\n");
        return -1;
    }

    if ((publicRsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL)) == NULL)
    {
        //printf("PEM_read_RSA_PUBKEY error \n");
        return -2;
    }
    fclose(fp);
    strcat(from, strsandwich);

    int rsa_len = RSA_size(publicRsa);
    int len = rsa_len - 11;
    if (RSA_public_encrypt(len, from, to, publicRsa, RSA_PKCS1_PADDING) < 0)
    {
        //printf("RSA_public_encrypt error\n");
        return -3;
    }
    RSA_free(publicRsa);
    return 0;
}

/*
 *  Decryption by RSA private key
 *  return -1: Private key path error
 *  retrun -2: PEM_read_RSAPrivateKey error
 *  return -3: RSA_private_decrypt error
 *  return 0 : Success        
 */
int RSA_decrypt_1024(char *from, char *to)
{
    FILE *fp = NULL;
    RSA *privateRsa = NULL;

    if ((fp = fopen(PRIVATEKEY, "r")) == NULL)
    {
        //printf("private key path error\n");
        return -1;
    }

    if ((privateRsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL)
    {
        //printf("PEM_read_RSAPrivateKey error\n");
        return -2;
    }
    fclose(fp);
    int rsa_len = RSA_size(privateRsa);
    if (RSA_private_decrypt(rsa_len, from, to, privateRsa, RSA_PKCS1_PADDING) < 0)
    {
        //printf("RSA_private_decrypt error\n");
        return -3;
    }
    RSA_free(privateRsa);
    return 0;
}

/*
 * For debug
 */
void printCipherText(char *target, int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("%X", (int)(*(target + i)));
    }
    printf("\n");
}

/*
 * Get the real string from remove salt.
 */
char *strJuicer(char *target)
{
    const char split[2] = "@@";
    char *token;
    token = strtok(target, split);
    return token;
}