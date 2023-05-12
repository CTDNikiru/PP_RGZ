#include <iostream>
#include <cstring>

using namespace std;

int X = 500;
int Y = 500;
int Z = 500;
int nThread = 8;
int ***total;

struct Z_size
{
    int Z_start = 0;
    int Z_end = 0;
};

void *thread_func(void *arg)
{
    Z_size *params = (Z_size *)arg;

    cout << "thread start" << params->Z_start << "to " << params->Z_end << endl;

    for (int z = params->Z_start; z < params->Z_end; z++)
    {
        for (int y = 0; y < Y; y++)
        {
            for (int x = 0; x < X; x++)
            {
                total[z][y][x] = rand();
            }
        }
    }
}

void start()
{
    int ***arr = new int **[Z];
    int err;

    for (int i = 0; i < Z; i++)
    {
        arr[i] = new int *[Y];

        for (int j = 0; j < Y; j++)
        {
            arr[i][j] = new int[X];
        }
    }
    total = arr;

    int *size = new int[nThread];
    Z_size *arr_size = new Z_size[nThread];

    // Массив потоков
    pthread_t *threads = new pthread_t[nThread];

    if (nThread == 1)
    {
        arr_size[0].Z_start = 0;
        arr_size[0].Z_end = Z-1;
    }
    else
    {
        int temp = Z / nThread;
        for (int i = 0; i < nThread - 1; i++)
        {
            arr_size[i].Z_start = temp * i;
            arr_size[i].Z_end = arr_size[i].Z_start + temp - 1;
        }

        arr_size[nThread - 1].Z_start = arr_size[nThread - 2].Z_end + 1;
        arr_size[nThread - 1].Z_end = Z - 1;
    }

    for (int i = 0; i < nThread; i++)
    {
        // Создание потока
        err = pthread_create(&threads[i], NULL, thread_func, (void *)&arr_size[i]);
        if (err != 0)
        {
            cout << "Cannot create a thread: " << strerror(err) << endl;
            exit(-1);
        }
    }

    // Ждем завершения всех созданных потоков
    for (int i = 0; i < nThread; ++i)
    {
        pthread_join(threads[i], NULL);
    }
}
