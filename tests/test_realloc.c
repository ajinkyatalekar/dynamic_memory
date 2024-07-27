#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    void *p1, *p2, *p3, *p4;

    p1 = malloc(5);
    p2 = realloc(p1, 10);

    if (p1 != p2)
    {
        return 1;
    }

    p3 = malloc(5);
    p4 = realloc(p3, 30);

    if (p3 == p4)
    {
        return 1;
    }

    return 0;
}