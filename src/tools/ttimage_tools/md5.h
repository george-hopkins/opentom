#ifndef _MD5_H_
#define _MD5_H_

struct MD5Context {
  unsigned int buf[4];
  unsigned int bits[2];
  unsigned char in[64];
};

typedef struct MD5Context MD5_CTX;

/* initialize a MD5 context */
void MD5Init(MD5_CTX *context);

/* add data to the hash */
void MD5Update(MD5_CTX *context, unsigned char const *buf, unsigned len);

/* finalize hash */
void MD5Final(unsigned char *digest, MD5_CTX *context);

#endif // _MD5_H_
