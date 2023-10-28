#ifndef MAIN_H
#define MAIN_H


// CONSTANTS
#define BITS    (int) 1024      // CHANGE THE NUMBER OF BITS HERE
#define LIMBS   (int) BITS/32  // Bits must be divisible by 32
#define K       (int) 40       // Number of rounds of Miller-Rabin

// VARIABLES
// Global prime to test
unsigned int n[LIMBS];
int num_processes = 6;
// Miller-Rabin variables
int r, found;
unsigned int s[LIMBS];
unsigned int a[LIMBS];
unsigned int x[LIMBS];
unsigned int nmin1[LIMBS];
unsigned int two[LIMBS] = {2}; // Elements with missing values are assigned 0
// Modpow variables
unsigned int result[LIMBS];
unsigned int base_copy[LIMBS];
unsigned int exponent_copy[LIMBS];
unsigned int mul_result[LIMBS*2]; // Shared with mul_two_bigints()
// Mod_two_bigints variables
unsigned int un[LIMBS*2+1];
unsigned int vn[LIMBS];
unsigned int quot[LIMBS+1];
unsigned int rem[LIMBS];

// Functions
unsigned long mix(unsigned long a, unsigned long b, unsigned long c);
unsigned int randint();
unsigned int randrange(int max);
void rand_bigint(int limbs, unsigned int x[limbs]);
void print_hex_bigint(unsigned int x[LIMBS]);
void copy_bigint(int limbs, unsigned int dest[limbs], unsigned int source[limbs]);
void zero_bigint(int limbs, unsigned int x[limbs]);
void shr1_bigint(unsigned int x[LIMBS]);
int cmp_int_and_bigint(unsigned int x[LIMBS], int y);
int cmp_two_bigints(unsigned int x[LIMBS], unsigned int y[LIMBS]);
void mul_two_bigints(unsigned int x[LIMBS], unsigned int y[LIMBS]);
void div_two_bigints(unsigned int dividend[LIMBS*2], unsigned int divisor[LIMBS]);
void add_int_to_bigint(unsigned int x[LIMBS], int y);
int bigint_is_nonzero(unsigned int x[LIMBS]);
void modpow_bigint(unsigned int base[LIMBS], 
                   unsigned int exponent[LIMBS], 
                   unsigned int modulus[LIMBS]);
int miller_rabin(unsigned int n[LIMBS]);
int main(void);

#endif