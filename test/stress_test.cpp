#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include "../skiplist.h"

using namespace std;

#define NUM_THREADS 1
#define TEST_COUNT 1000000

Skiplist<int, string> skiplist(18);

void* insert(void* threadid) {
    long tid;
    tid = (long) threadid;
    cout << tid << endl;
    int tmp = TEST_COUNT / NUM_THREADS;
    for (int i = tid * tmp, count = 0; count < tmp; i++) {
        skiplist.insert_element(rand(), "a");
        count++;
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    {
        pthread_t threads[NUM_THREADS];
        int ret;
        int i;

        auto start = chrono::high_resolution_clock::now();

        for (i = 0; i < NUM_THREADS; i++) {
            cout << "Creating thread: " << i << endl;

            ret = pthread_create(&threads[i], NULL, insert, (void*)i);
            if (ret) {
                cout << "Error: failed to create thread," << ret << endl;
                exit(-1);
            }
        }

        void* r;
        for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_join(threads[i], &r) != 0) {
                perror("pthread_create() error");
                exit(3);
            }
        }

        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;
        cout << "insert elapsed: " << elapsed.count() << endl;
    }
    pthread_exit(NULL);
    return 0;
}