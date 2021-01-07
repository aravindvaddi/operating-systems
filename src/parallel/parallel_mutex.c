#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_BUCKETS 5     // Buckets in hash table
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
int keys[NUM_KEYS];

pthread_mutex_t lock[NUM_BUCKETS];

typedef struct _bucket_entry {
    int key;
    int val;
    struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];

void panic(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {

    /*
        Below line added as a part of step 1 for a thread to acquire the mutux lock, but replaced with array of locks with an entry for each bucket for step 4
        It added an overhead for insert and increased the insert times by about 15% for 1 thread, 60% for 5 and 10 threads, 36% for 100 threads as compared to the given parallel_hashtable.c 
    */
    // pthread_mutex_lock(&lock);

    /*
        Below line added as a part of step 4 so access to individual buckets is made thread safe as opposed to the whole hashtable so some insert operations can run in parallel
        It increased insert time for 1 tread by 42% but decreased for 5, 10 and 100 threads by about 4%, 14%, 11% respectively from step 1.
    */
    pthread_mutex_lock(&lock[key % NUM_BUCKETS]);

    int i = key % NUM_BUCKETS;
    bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));
    if (!e) panic("No memory to allocate bucket!");
    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

    // Below line added as a part of step 4 so access to individual buckets is made thread safe as opposed to the whole hashtable so some insert operations can run in parallel
    pthread_mutex_unlock(&lock[key % NUM_BUCKETS]);

    // Below line added as a part of step 1 for a thread to release the mutux lock, but replaced with array of locks with an entry for each bucket for step 4
    // pthread_mutex_unlock(&lock);

}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {
    bucket_entry *b;
    for (b = table[key % NUM_BUCKETS]; b != NULL; b = b->next) {
        if (b->key == key) return b;
    }
    return NULL;
}

void * put_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;

    // If there are k threads, thread i inserts
    //      (i, i), (i+k, i), (i+k*2)
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        insert(keys[key], tid);
    }

    pthread_exit(NULL);
}

void * get_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    long lost = 0;

    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        /*
            Below line added as part of Step 2 to acquire a spin lock by a thread - it is not required since while reading the memory is not altered and the data is thread safe by default
            It was removed as part of step 3
        */
        // pthread_mutex_lock(&lock);

        if (retrieve(keys[key]) == NULL) lost++;

        /*
            Below line added as part of Step 2 to acquire a spin lock by a thread - it is not required since while reading the memory is not altered and the data is thread safe by default
            It was removed as part of step 3
        */
        // pthread_mutex_unlock(&lock);
    }
    printf("[thread %ld] %ld keys lost!\n", tid, lost);

    pthread_exit((void *)lost);
}

int main(int argc, char **argv) {
    long i;
    pthread_t *threads;
    double start, end;
    int c;

    if (argc != 2) {
        panic("usage: ./parallel_hashtable <num_threads>");
    }
    if ((num_threads = atoi(argv[1])) <= 0) {
        panic("must enter a valid number of threads to run");
    }

    srandom(time(NULL));
    for (i = 0; i < NUM_KEYS; i++)
        keys[i] = random();

    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    if (!threads) {
        panic("out of memory allocating thread handles");
    }

    for (c = 0; c < NUM_BUCKETS; c++) {
        if (pthread_mutex_init(&lock[c], NULL) != 0) {
            panic("mutux init has failed\n");
        }
    }

    // Insert keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, put_phase, (void *)i);
    }

    // Barrier
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end = now();

    printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);

    // Reset the thread array
    memset(threads, 0, sizeof(pthread_t)*num_threads);

    // Retrieve keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, get_phase, (void *)i);
    }

    // Collect count of lost keys
    long total_lost = 0;
    long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void **)&lost_keys[i]);
        total_lost += lost_keys[i];
    }
    end = now();

    printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

    return 0;
}
