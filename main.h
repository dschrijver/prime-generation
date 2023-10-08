#ifndef MAIN_H
#define MAIN_H


#define BITS (int) 1024
#define BYTES (int) (BITS / 8)
#define LIMBS (int) (BITS / 32)
#define DOUBLE_LIMBS (int) (LIMBS * 2)

#define K (int) 40

unsigned int randint();
void bigrandint(int limbs, unsigned int n[limbs]); 

void printbighex(int limbs, unsigned int n[limbs]);

void randrangebig(int limbs, unsigned int n_max[limbs], unsigned int a[limbs]);

void bigintcopy(int limbs, unsigned int source[limbs], unsigned int destination[limbs]);
int bigintnotzero(int limbs, unsigned int a[limbs]);

void mul2bigints(int limbs, unsigned int a[limbs], unsigned int b[limbs], unsigned int y[2*limbs]);
void shr1bigint(int limbs, unsigned int a[limbs]);
void subintbigint(unsigned int b, int limbs, unsigned int source[limbs],  unsigned int destination[limbs]);
void addintbigint(unsigned int b, int limbs, unsigned int source[limbs],  unsigned int destination[limbs]);
int cmpbigints(int limbs, unsigned int a[limbs], unsigned int b[limbs]);

int nlz(unsigned int x);
void div2bigints(int ul, int vl, unsigned int u[ul], unsigned int v[vl], unsigned int q[ul-vl+1], unsigned int r[vl]);
void modpow(int limbs, unsigned int base[limbs], unsigned int exponent[limbs], unsigned int modulus[limbs],
            unsigned int result[limbs]);

int miller_rabin(int limbs, unsigned int n[limbs]);

int main(int argc, char *argv[]);

#endif