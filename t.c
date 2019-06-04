#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
    int height = 11;
    int width = 11;
    int n = 2;
    int num_per_height = floor(height / n);
    int num_per_width = floor(width / n);
    int arr[4];

    int n_current = 1;
    int j = n_current * num_per_height;

    arr[0] = j;
    if (n_current + 1 == n) {
        arr[1] = height - 1;
    } else {
        arr[1] = (j + num_per_height) - 1;
    }

    printf("y1:%d y2:%d\n", arr[0], arr[1]);
    return 0;

    //n_current = 0;
    //for (int i = 0; i < width; i += num_per_width) {
    //    arr[n_current][2] = i;
    //    if (n_current + 1 == n) {
    //        arr[n_current][3] = width - 1;
    //    } else {
    //        arr[n_current][3] = (i + num_per_width) - 1;
    //    }
    //    n_current++;
    //}

    //for (int i = 0; i < n; i++) {
    //    printf("y1:%d y2:%d x1:%d x2:%d\n", arr[i][0], arr[i][1], arr[i][2], arr[i][3]);
    //}

    return 0;
}
