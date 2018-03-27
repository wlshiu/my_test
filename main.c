#include<stdio.h>
#include<stdlib.h>


int way[1001] = {0};
int n, m;

void make (int now, int a, int n, int m)
{
    int b = a, c;
    if(now == m + 1)
    {
        for(c = 1; c <= m; c++)
            printf("%d ", way[c]);
        printf("\n");
        return;
    }
    else
    {
        for(b = a; b <= n; b++)
        {
            way[now] = b;
            make(now + 1, b + 1, n, m);
        }
    }
    return;
}


int main(void)
{
//    while(scanf("%d %d", &n, &m) == 2)
//        make(1, 1, n, m);

    make(1, 1, 6, 2);
    return 0;
}
