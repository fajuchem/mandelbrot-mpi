#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
    int height = 10;
    int width = 10;
    int n = 4;

    int columns = height / ceil(sqrt(n));
    int rows = width / ceil(sqrt(n));

    int arr[n][4];

    int count = 0;
    for (int j = 0; j < height; j += columns) {
        for (int i = 0; i < width; i += rows) {
            arr[count][0] = j + columns;
            arr[count][1] = j;
            arr[count][2] = i + rows;
            arr[count][3] = i;
            count++;
        }
    }

    for (int i = 0; i < n; i++) {
        printf("y2:%d y1:%d x2:%d x1:%d\n", arr[i][0], arr[i][1], arr[i][2], arr[i][3]);
    }

    //printf("y1:%d y2:%d\n", arr[0], arr[1]);

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
