#include <stdio.h> // printf()
#include <stdlib.h> // srand(), rand()
#include <time.h> // time()

#include "main.h"


unsigned int randint() {
    // rand() gives value in [0, RAND_MAX], with RAND_MAX = 2^31-1. 
    // This means that rand() gives 31 random bits. 
    // Use bottom 16 bits twice to get 32 random bits. 
    unsigned int x = rand() & 0xffff;
    x |= (rand() & 0xffff) << 16;
    return x;
}


void bigrandint(int limbs, unsigned int n[limbs]) {
    for (int i = 0; i < limbs; i++) {
        n[i] = randint();
    }
    return;
}


void printbighex(int limbs, unsigned int n[limbs]) {
    for (int i = limbs - 1; i >= 0; i--) {
        printf("%08x", n[i]); // x option prints unsigned hex
        // printf(" ");
    }
    return;
}


// Gives a big int a in [0, n_max). 
void randrangebig(int limbs, unsigned int n_max[limbs], unsigned int a[limbs]) {
    bigrandint(limbs, a);
    unsigned int end, r;
    // Check all limbs of a
    for (int i = limbs - 1; i >= 0; i--) {
        if (a[i] < n_max[i]) {
            break;
        }
        // For the skew
        end = 0xffffffff / n_max[i]; 
        end *= n_max[i]; 
        while ((r = randint()) >= end); // Should take only a few loops

        a[i] = r % n_max[i]; // a[i] < n_max[i]
    }

    return;
}


void bigintcopy(int limbs, unsigned int source[limbs], unsigned int destination[limbs]) {
    for (int i = 0; i < limbs; i++) {
        destination[i] = source[i];
    }
    return;
}


int bigintnotzero(int limbs, unsigned int a[limbs]) {
    for (int i = 0; i < limbs; i++) {
        if(a[i] > 0) {
            return 1;
        }
    }
    return 0;
}


// multiply two ints with the same amount of limbs.
// y must be initialized with zeros. 
void mul2bigints(int limbs, unsigned int a[limbs], unsigned int b[limbs], unsigned int y[2*limbs]) {
    unsigned long mul;
    int k; 
    unsigned int previous, carry;
    for (int i = 0; i < limbs; i++) {
        for (int j = 0; j < limbs; j++) {
            mul = (unsigned long)a[i]*(unsigned long)b[j];

            // Lower limb
            previous = y[i+j];
            y[i+j] += (unsigned int) (mul & 0xffffffff);
            carry = y[i+j] < previous;
            k = i+j+1;
            while (carry && (k < 2*limbs)) {
                previous = y[k];
                y[k] += (unsigned int) 1;
                carry = y[k] < previous;
                k++;
            }
            // Higher limb
            previous = y[i+j+1];
            y[i+j+1] += (unsigned int) (mul >> 32);
            carry = y[i+j+1] < previous;
            k = i+j+2;
            while (carry && (k < 2*limbs)) {
                previous = y[k];
                y[k] += (unsigned int) 1;
                carry = y[k] < previous;
                k++;
            }
        }
    }
    return;
}


void shr1bigint(int limbs, unsigned int a[limbs]) {
    for (int i = 0; i<limbs-1; i++) {
        a[i] = (a[i] >> 1) | (a[i+1] << 31);
    }
    a[limbs-1] = a[limbs-1] >> 1;
}


void subintbigint(unsigned int b, int limbs, unsigned int source[limbs],  unsigned int destination[limbs]) {
    unsigned int previous;
    int borrow;
    int i = 0;
    previous = source[i];
    destination[i] = source[i] - b;
    borrow = destination[i] > previous;
    i++;
    while (i < limbs) {
        previous = source[i];
        destination[i] = source[i] - borrow;
        borrow = destination[i] > previous;
        i++;
    }
    return;
}


void addintbigint(unsigned int b, int limbs, unsigned int source[limbs],  unsigned int destination[limbs]) {
    unsigned int previous;
    int carry;
    int i = 0;
    previous = source[i];
    destination[i] = source[i] + b;
    carry = destination[i] < previous;
    i++;
    while (i < limbs) {
        previous = source[i];
        destination[i] = source[i] + carry;
        carry = destination[i] < previous;
        i++;
    }
    return;
}


int cmpbigints(int limbs, unsigned int a[limbs], unsigned int b[limbs]) {
    for (int i = 0; i < limbs; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}


int nlz(unsigned int x) {
    int n;
    if (x == 0) return (int)32;
    n = 0;
    if (x <= 0x0000ffff) {n = n + 16; x = x << 16;}
    if (x <= 0x00ffffff) {n = n + 8; x = x << 8;}
    if (x <= 0x0fffffff) {n = n + 4; x = x << 4;}
    if (x <= 0x3fffffff) {n = n + 2; x = x << 2;}
    if (x <= 0x7fffffff) {n = n + 1;}
    return n;
}


void div2bigints(int ul, int vl, unsigned int u[ul], unsigned int v[vl], unsigned int q[ul-vl+1], unsigned int r[vl]) {
    int m = ul - vl; 
    int n = vl;
    unsigned long b = (unsigned long)4294967296; // 2^32

    unsigned long p;
    signed long t, k;

    int s = nlz(v[n-1]);

    unsigned int vn[n], un[m+n+1];

    if (s == 0) {
        for (int i = 0; i < n; i++) {
            vn[i] = v[i];
        }
        un[m+n] = 0;
        for (int i = 0; i < m+n; i++) {
            un[i] = u[i];
        }
    } else {
        // TODO, not necessary in my usecase
    }

    unsigned long qhat, rhat;
    for (int j = m; j >= 0; j--) {
        qhat = (((unsigned long)un[n+j]<<32)+(unsigned long)un[n-1+j])/(unsigned long)vn[n-1];
        rhat = (((unsigned long)un[n+j]<<32)+(unsigned long)un[n-1+j])%(unsigned long)vn[n-1];
        
        // rhat < b per definition.
        while ((qhat >= b) || 
            (qhat * (unsigned long)vn[n-2]) > ((rhat<<32) + (unsigned long)un[n-2+j])) {
            qhat = qhat - 1;
            rhat = rhat + (unsigned long)vn[n-1];
            if(rhat >= b) {
                break;
            }
        }

        k = 0;
        for (int i = 0; i < n; i++) {
            p = qhat * (unsigned long)vn[i];
            // subtract the lower limb of p from un[i+j]
            t = (unsigned long)un[i+j] - k - (unsigned long)(p & (unsigned long)0xFFFFFFFF);
            un[i+j] = (unsigned int)(t & (unsigned long)0xFFFFFFFF);
            k = (unsigned long)(p >> 32) - (unsigned long)(t >> 32);
        }
        t = (unsigned long)un[n+j] - k;
        un[n+j] = (unsigned int)(t & (signed long)0xFFFFFFFF);

        q[j] = (unsigned int)qhat;
        if (t < 0) {
            // printf("t < 0, j = %d\n", j);
            q[j] = q[j]-1;
            k = 0;
            for (int i = 0; i < n; i++) {
                t = (signed long)un[i+j] + (signed long)vn[i] + k;
                un[i+j] = (unsigned int)(t & (signed long)0xFFFFFFFF);
                k = (signed long) (t >> 32); // higher limb of k is zero
            }
            un[n+j] = un[n+j] + (unsigned int)k; // only lower limb of k is nonzero
        }
    }

    if (s == 0) {
        for (int i = 0; i < n; i++) {
            r[i] = un[i];
        }
    } else {
        // TODO, not necessary in my usecase
    }
    return;
}


void modpow(int limbs, unsigned int base[limbs], unsigned int exponent[limbs], unsigned int modulus[limbs],
            unsigned int result[limbs]) {
    unsigned int result_copy[limbs];
    // TODO: base %= modulus, not necessary in my usecase. 
    for (int i = 1; i < limbs; i++) {
        result_copy[i] = 0;
    }
    result_copy[0] = 1;
    unsigned int base_copy[limbs], exponent_copy[limbs];
    bigintcopy(limbs, base, base_copy);
    bigintcopy(limbs, exponent, exponent_copy);
    
    unsigned int mul[2*limbs];
    unsigned int q[limbs];

    while (bigintnotzero(limbs, exponent_copy)) {
        if ((exponent_copy[0] % 2) == 1) {
            for (int i = 0; i < 2*limbs; i++) {
                mul[i] = 0;
            }
            mul2bigints(limbs, result_copy, base_copy, mul);
            div2bigints(2*limbs, limbs, mul, modulus, q, result_copy);
        }
        shr1bigint(limbs, exponent_copy);
        for (int i = 0; i < 2*limbs; i++) {
            mul[i] = 0;
        }
        mul2bigints(limbs, base_copy, base_copy, mul);
        div2bigints(2*limbs, limbs, mul, modulus, q, base_copy);
    }

    for (int i = 0; i < limbs; i++) {
        result[i] = result_copy[i];
    }

    return;
}


int miller_rabin(int limbs, unsigned int n[limbs]) {
    int r = 0;
    unsigned int s[limbs], nm1[limbs], nm3[limbs], one[limbs], two[limbs];
    subintbigint(1, limbs, n, s);
    subintbigint(1, limbs, n, nm1);
    subintbigint(3, limbs, n, nm3);
    for (int i = 1; i < limbs; i++) {
        one[i] = 0;
        two[i] = 0;
    }
    one[0] = 1;
    two[0] = 2;

    while ((s[0]%2) == 0) {
        r++;
        shr1bigint(limbs, s);
    }

    unsigned int a[limbs], x[limbs];
    int found;

    for (int i = 0; i < K; i++) {
        randrangebig(limbs, nm3, a);
        addintbigint(2, limbs, a, a);

        modpow(limbs, a, s, n, x);

        if (cmpbigints(limbs, x, one) || cmpbigints(limbs, x, nm1)) {
            continue;
        }

        found = 0;
        for (int j = 0; j < r-2; j++) {
            modpow(limbs, x, two, n, x);
            
            if (cmpbigints(limbs, x, nm1)) {
                found = 1;
                break;
            }
        }

        if(!found) {
            return 0;
        }
    }
    return 1;
}


int main(int argc, char *argv[]) {
    srand(time(NULL));

    printf("Number of limbs (32bits) in n: %d\n", LIMBS);

    unsigned int n[LIMBS];
    bigrandint(LIMBS, n);
    // Make sure n is the right amount of bits (highest bit is set)
    // and is odd
    n[LIMBS - 1] |= 0x80000000; // Highest bit is set
    n[0] |= 1; // Lowest bit is set

    while (1) {
        printf("n = ");
        printbighex(LIMBS, n);
        printf("\n");

        int isprime = miller_rabin(LIMBS, n);
        if (isprime) {
            printf("Prime!\n");
            break;
        } else {
            printf("not prime...\n");
        }
        addintbigint(2, LIMBS, n, n);
    }

    return 0;
}