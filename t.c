#include <stdio.h>
#include <math.h>

int main(int agrc, char *argv[])
{
    int h = 5;
    int w = 8;
    int p = 3;

    int mat[5][8] = {
        { 1, 2, 3, 4, 5, 6, 7, 8 },
        { 1, 2, 3, 4, 5, 6, 7, 8 },
        { 1, 2, 3, 4, 5, 6, 7, 8 },
        { 1, 2, 3, 4, 5, 6, 7, 8 },
        { 1, 2, 3, 4, 5, 6, 7, 8 }
    };

    int size_h = h / p;
    int left_size_h = h % p;
    int size_w = w / p;
    int left_size_w = w % p;

    printf("size_h: %d\n", size_h);
    printf("left_size_h: %d\n", left_size_w);
    printf("size_w: %d\n", size_w);
    printf("left_size_w: %d\n", left_size_w);

    return 0;
}
