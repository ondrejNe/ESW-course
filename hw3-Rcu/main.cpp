#include "list.hpp"
#include "definitions.hpp"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <vector>
#include <atomic>
#include <string>
#include <array>
#include <iostream>
#include <ostream>
#include "signal.h"

#if !defined(USE_MUTEX) && !defined(USE_RWLOCK) && !defined(USE_RCU)
#error "Compile this with one of the USE_* defines!"
#endif

#define LIST_LENGTH 100
#define KEY_LEN 4
#define VALUE_LEN 40
#define WRITERS_COUNT 1
#define WRITE_PERIOD_US 1000

using namespace std;

constexpr auto relaxed = memory_order_relaxed;

constexpr char charset[] = "abcdefghijklmnopqrstuvwxyz";

static LIST_TYPE list;

atomic<bool> terminate_req = false;

static __thread struct stats {
    atomic<unsigned long> reads;
    atomic<unsigned long> writes;
} stats;

mutex all_stats_mutex;
vector<struct stats*> all_stats;

static array<string, LIST_LENGTH> used_keys;

// Per-thread seed for rand_r(). Using rand() would cause slow cache
// collisions on shared seed.
__thread unsigned int seed;

void generate_random_string(char *str, size_t length)
{
    int len = strlen(charset);
    for (size_t i = 0; i < length - 1; i++) {
        int r = rand_r(&seed) % len;
        str[i] = charset[r];
    }
    str[length - 1] = '\0';
}

void init_list()
{
    char key[KEY_LEN + 1];
    char value[VALUE_LEN + 1];
    esw_list_init(&list);
    for (auto &used_key: used_keys) {
        generate_random_string(key, sizeof(key));
        generate_random_string(value, sizeof(value));

        esw_list_push(&list, key, value);
        used_key = key;
    }
}

void *reader_thread(void *arg)
{
    char key[KEY_LEN + 1];
    char address[VALUE_LEN + 1];

    {
        // Store pointers to our per-thread statistics to the global variable
        lock_guard<mutex> lock(all_stats_mutex);
        all_stats.push_back(&stats);

        // Initialize per-thread random seed from global RNG
        seed = rand();
    }

    long hits = 0;
    long miss = 0;

#if defined(USE_RCU)
    rcu_register_thread();
#endif

    while (!terminate_req.load(relaxed)) {
        generate_random_string(key, sizeof(key));

        if (esw_list_find(&list, key, address, sizeof(address)) == true) {
            hits++;
        } else {
            miss++;
        }

        // Update the per-thread counter safely but without an atomic
        // instruction (note that no other thread modifies this)
        stats.reads.store(stats.reads.load(relaxed) + 1, relaxed);
    }
    printf("Hits: %ld, Misses: %ld\n", hits, miss);

#if defined(USE_RCU)
    rcu_unregister_thread();
#endif
    return NULL;
}

void *writer_thread(void *arg)
{
    {
        // Store pointers to our per-thread statistics to the global variable
        lock_guard<mutex> lock(all_stats_mutex);
        all_stats.push_back(&stats);
    }
    while (!terminate_req.load(relaxed)) {
        char new_value[VALUE_LEN + 1];
        generate_random_string(new_value, sizeof(new_value));
        esw_list_update(&list,
                        used_keys[rand_r(&seed) % LIST_LENGTH].c_str(),
                        new_value);

        usleep(WRITE_PERIOD_US);

        // Update the per-thread counter safely but without an atomic
        // instruction (note that no other thread modifies this)
        stats.writes.store(stats.writes.load(relaxed) + 1, relaxed);
    }
    return NULL;
}

void run_and_print_stats()
{
    long reads_local_last = 0;
    long writes_local_last = 0;
    sleep(1);
    while (!terminate_req) {
        long sum_reads = 0, sum_writes = 0;
        {
            lock_guard<mutex> lock(all_stats_mutex);
            for (auto thr : all_stats) {
                sum_reads += thr->reads.load(relaxed);
                sum_writes += thr->writes.load(relaxed);
            }
        }
        printf("Reads: %-10ld  Writes: %-10ld\n",
               sum_reads - reads_local_last,
               sum_writes - writes_local_last);
        reads_local_last = sum_reads;
        writes_local_last = sum_writes;
        fflush(stdout);

        sleep(1);
    }
}

void sig_handler(int)
{
    terminate_req = true;
}

int main(int argc, char *argv[])
{
    int reader_count;

    if (argc != 2)
        errx(EXIT_FAILURE, "usage: %s <reader-thread quantity>", argv[0]);

    reader_count = atoi(argv[1]);

    if (reader_count < 1 || reader_count > 100)
        errx(EXIT_FAILURE, "min readers: 1, max readers: 100");

    printf("Size of one list element: %zu\n", sizeof(struct esw_node));

    init_list();

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    vector<pthread_t> readers(reader_count);
    vector<pthread_t> writers(WRITERS_COUNT);

    for (int i = 0; i < reader_count; i++)
        if (pthread_create(&readers[i], NULL, reader_thread, NULL) != 0)
            err(1, "pthread_create reader");

    for (int i = 0; i < WRITERS_COUNT; i++)
        if (pthread_create(&writers[i], NULL, writer_thread, NULL) != 0)
            err(1, "pthread_create writer");

    run_and_print_stats();

    terminate_req = true;

    for (int i = 0; i < WRITERS_COUNT; i++)
        pthread_join(writers[i], NULL);

    for (int i = 0; i < reader_count; i++)
        pthread_join(readers[i], NULL);

    esw_list_free_content(&list);

    return EXIT_SUCCESS;
}
