#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    void *p1, *p2, *p3, *brk1, *brk2;

    p1 = malloc(2040);
    brk1 = sbrk(0);
    free(p1);

    p2 = malloc(2040);
    p3 = malloc(2040);
    brk2 = sbrk(0);

    printf("%p %p %p\n", p1, p2, p3);
    if (brk1 != brk2)
    {
        return 1;
    }

    void *q1, *q2, *b1, *b2;
    q1 = malloc(4088);
    b1 = sbrk(0);
    free(q1);

    q2 = malloc(4088);
    b2 = sbrk(0);
    free(q2);

    if (b1 != b2) {
	return 1;
    }

    return 0;
	    
}
