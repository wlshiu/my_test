
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 *  N choose K
 *  C(N, K) = P(N, K)/K! = N!/((N - K)! * K!);
 *  C(N, K) = C(N, N - K)
 */
static int factorial(int x)
{
    int     s = 1;
    for(int i = 1; i <= x; i++)
        s *= i;
    return s;
}

static int
next_comb(int *pComb, const int n, const int k)
{
	int     i = k - 1;
	const int e = n - k;

	do {
		pComb[i]++;
	} while( pComb[i] > e + i && i-- );

	if( pComb[0] > e )
		return 0;

	while( ++i < k )
		pComb[i] = pComb[i - 1] + 1;

	return 1;
}

int main()
{
    int     rval = 0;
    int     *pComb = 0;
    int     N, K;

    printf("N choose K elements, C(N, M): N K = \n");
    scanf("%d %d", &N, &K);
    printf("A combination of %d elements taken %d at a time without repetition: \n", N, K);

    do {
        if( N < K || K <= 0 )
        {
            printf("wrong parameters \n");
            rval = -1;
            break;
        }

        if( !(pComb = malloc(sizeof(int) * K)) )
        {
            printf("malloc %d fail \n", K);
            rval = -1;
            break;
        }

        for(int i = 0; i < K; i++)
            pComb[i] = i;

        do {
            for(int i = 0; i < K; ++i)
            {
                printf("%d ", pComb[i] + 1);
            }

            printf("\n");

        } while( next_comb(pComb, N, K) );

        printf("Number of combinations = %d\n\n", factorial(N)/ (factorial(N - K) * factorial(K)));

    } while(0);

    if( pComb )     free(pComb);


    system("pause");
    return rval;
}
