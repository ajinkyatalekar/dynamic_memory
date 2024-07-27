#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    void *p1, *p2, *p3, *p4, *p5, *p6;

    p1 = malloc(48);
    p2 = malloc(81);
    p3 = malloc(48);
    p4 = malloc(100);

    if (!(p1 + 64 == p3 && p2 + 128 == p4))
    {
        return 1;
    }

    p5 = malloc(2);
    p6 = malloc(1);

    if (!(p5 + 32 == p6))
    {
        return 1;
    }

    void *q1, *q2;

    q1 = malloc(4088);
    free(q1);
    q2 = malloc(4088);
    free(q2);

    void *r1, *r2;
    r1 = malloc(4089);
    free(r1);
    r2 = malloc(4089);
    free(r2);

    return 0;
}
