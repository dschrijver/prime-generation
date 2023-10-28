#include <stdio.h> // printf()
#include <stddef.h> // size_t
#include <stdlib.h> // srand(), rand() 
#include <time.h> // time(), clock()
#include <unistd.h> // fork(), getpid()

#include "main.h"


unsigned long mix(unsigned long a, unsigned long b, unsigned long c) {
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

unsigned int randint() {
    // rand() gives value in [0, RAND_MAX], with RAND_MAX = 2^31-1. 
    // This means that rand() gives 31 random bits. 
    // Use first rand() for highest 31 bits, second rand() for lowest bit. 
    unsigned int x = (rand() & 0x7fffffff) << 1;
    x |= (rand() & 1);
    return x;
}

// Returns an integer in range [0, max) whilst taking skew into account
unsigned int randrange(int max) {
    unsigned int end, r;
    end = 0xffffffff / max; 
    end *= max;
    while ((r = randint()) >= end); // Should take only a few loops
    return r % max; 
}

void rand_bigint(int limbs, unsigned int x[limbs]) {
    for (int i = 0; i < limbs; i++) {
        x[i] = randint();
    }
    return;
}

void print_hex_bigint(unsigned int x[LIMBS]) {
    // printf("0x ");
    printf("0x");
    for (int i = LIMBS-1; i >= 0; i--) {
        printf("%08x", x[i]);
        // if (i > 0) {
        //     printf(" ");
        // }
    }
    printf("\n");
    return;
}

void copy_bigint(int limbs, unsigned int dest[limbs], unsigned int source[limbs]) {
    for (int i = 0; i < limbs; i++) {
        dest[i] = source[i];
    }
    return;
}

void zero_bigint(int limbs, unsigned int x[limbs]) {
    for (int i = 0; i < limbs; i++) {
        x[i] = 0;
    }
    return;
}

void shr1_bigint(unsigned int x[LIMBS]) {
    for (int i = 0; i < (LIMBS-1); i++) {
        x[i] = (x[i] >> 1) | (x[i+1] << 31);
    }
    x[LIMBS-1] >>= 1;
    return;
}

int cmp_int_and_bigint(unsigned int x[LIMBS], int y) {
    if ((unsigned int)y != x[0]) {
        return 0;
    }
    for (int i = 1; i < LIMBS; i++) {
        if (x[i] != 0) {
            return 0;
        }
    }
    return 1;
}

int cmp_two_bigints(unsigned int x[LIMBS], unsigned int y[LIMBS]) {
    for (int i = 0; i < LIMBS; i++) {
        if (x[i] != y[i]) {
            return 0;
        }
    }
    return 1;
}

// Multiply two bigints with the same amount of limbs
void mul_two_bigints(unsigned int x[LIMBS], unsigned int y[LIMBS]) {
    unsigned int mul_old, carry;
    unsigned long mul;
    zero_bigint(LIMBS*2, mul_result);
    for (int i = 0; i < LIMBS; i++) {
        for (int j = 0; j < LIMBS; j++) {
            mul = (unsigned long)x[i]*(unsigned long)y[j];

            // Add lower limb to mul_result
            carry = (unsigned int)(mul & 0xffffffffUL);
            for (int k = i+j; k < (LIMBS*2); k++) {
                mul_old = mul_result[k];
                mul_result[k] += carry;
                if (mul_old <= mul_result[k]) break;
                carry = 1;
            }

            // Add higher limb to mul_result
            carry = (unsigned int)(mul >> 32);
            for (int k = i+j+1; k < (LIMBS*2); k++) {
                mul_old = mul_result[k];
                mul_result[k] += carry;
                if (mul_old <= mul_result[k]) break;
                carry = 1;
            }
        }
    }
    return;
}

// Divide two bigints, dividend of length limbs*2, divisor length limbs.
// Returns quotient and remainder.
void div_two_bigints(unsigned int dividend[LIMBS*2], unsigned int divisor[LIMBS]) {
    // No normalization is needed
    // Because the divisor has the highest bit set. 
    copy_bigint(LIMBS*2, un, dividend);
    copy_bigint(LIMBS, vn, divisor);

    int n = LIMBS;
    unsigned long b = 4294967296UL;
    unsigned long qhat, rhat, p;
    signed long t, k;
    for (int j = LIMBS; j >= 0; j--) {
        qhat = (((unsigned long)un[j+n] << 32) + (unsigned long)un[j+n-1]) / 
               ((unsigned long) vn[n-1]);
        rhat = (((unsigned long)un[j+n] << 32) + (unsigned long)un[j+n-1]) % 
               ((unsigned long) vn[n-1]); // Per definition < b
again:
        if ((qhat >= b) || 
            ((qhat * (unsigned long)vn[n-2]) > ((rhat << 32) + un[j+n-2]))) {
            qhat -= 1;
            rhat += (unsigned int)vn[n-1];
            if (rhat < b) goto again;
        }

        k = 0;
        for (int i = 0; i < n; i++) {
            p = qhat * (unsigned long)vn[i];
            t = (signed long)un[i+j] - k - (signed long)(p & 0xFFFFFFFFUL);
            un[i+j] = (unsigned int)t;
            k = (signed long)(p >> 32) - (t >> 32);
        }
        t = (unsigned long)un[j+n] - k;
        un[j+n] = t;
        
        quot[j] = qhat;
        if (t < 0) {
            quot[j] -= 1;
            k = 0;
            for (int i = 0; i < n; i++) {
                t = (signed long)un[i+j] + (signed long)vn[i] + k;
                un[i+j] = (unsigned int)t;
                k = t >> 32;
            }
            un[j+n] = un[j+n] + (unsigned int)k;
        }
    }
    copy_bigint(LIMBS, rem, un);
    return;
}

// Adds an integer to a bigint, assuming the bigint will not overflow
void add_int_to_bigint(unsigned int x[LIMBS], int y) {
    unsigned int xold;
    unsigned int carry = y;
    for (int i = 0; i < (LIMBS-1); i++) {
        xold = x[i];
        x[i] += carry;
        if (xold < x[i]) return;
        carry = 1;
    }
    x[LIMBS-1] += carry;
    return;
}

int bigint_is_nonzero(unsigned int x[LIMBS]) {
    for (int i = 0; i < LIMBS; i++) {
        if (x[i] != 0) {
            return 1;
        }
    }
    return 0;
}

void modpow_bigint(unsigned int base[LIMBS], 
                   unsigned int exponent[LIMBS], 
                   unsigned int modulus[LIMBS]) {
    zero_bigint(LIMBS, result);
    result[0] = 1;
    copy_bigint(LIMBS, base_copy, base);
    copy_bigint(LIMBS, exponent_copy, exponent);

    while (bigint_is_nonzero(exponent_copy)) {
        if ((exponent_copy[0] % 2) == 1) {
            mul_two_bigints(result, base_copy);
            div_two_bigints(mul_result, modulus);
            copy_bigint(LIMBS, result, rem);
        }

        shr1_bigint(exponent_copy);

        mul_two_bigints(base_copy, base_copy);
        div_two_bigints(mul_result, modulus);
        copy_bigint(LIMBS, base_copy, rem);
    }

    return;
}

int miller_rabin(unsigned int n[LIMBS]) {
    copy_bigint(LIMBS, nmin1, n);
    nmin1[0] &= 0xfffffffe;
    
    r = 0;
    copy_bigint(LIMBS, s, n);
    s[0] &= 0xfffffffe; // s = n-1

    while ((s[0] % 2) == 0) {
        r++;
        shr1_bigint(s);
    }

    for (int i = 0; i < K; i++) {
        // Generate a random bigint in [2, n-2]
        // First generate random bigint in [0, n-4]
        // Then add 2
        rand_bigint(LIMBS-1, a); // Lower limbs can be random.
        // Highest limb <= n-4
        a[LIMBS-1] = randrange(n[LIMBS-1]-3);
        add_int_to_bigint(a, 2);

        modpow_bigint(a, s, n);
        copy_bigint(LIMBS, x, result);

        if (cmp_int_and_bigint(x, 1) || cmp_two_bigints(x, nmin1)) {
            continue;
        }
        found = 0;
        for (int j = 0; j < (r-1); j++) {
            modpow_bigint(x, two, n);
            if (cmp_two_bigints(x, nmin1)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            return 0;
        }
    }
    return 1;
}


int main(void) {
    // Prime must have a multiple of 32 bits.
    if ((BITS % 32) != 0) {
        printf("Number of bits must be a multiple of 32.\n");
        return 1;
    }
    printf("Number of bits: %i\n", BITS);
    printf("Started searching for primes...\n");

    // Start infinitely running processes
    for (int i = 0; i < (num_processes-1); i++) { 
        if (fork() == 0) {
            break;
        }
    }

    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand(seed);

    while (1) {
        // Generate random odd number with maximal amount of bits
        rand_bigint(LIMBS, n);
        n[0] |= 1; // Make n odd
        n[LIMBS-1] |= 0x80000000; // Make n the maximal amount of bits
        if (miller_rabin(n)) {
            printf("\nPRIME!!!\n");
            printf("n = ");
            print_hex_bigint(n);
            fflush(stdout);
        }
    }

    return 0;
}