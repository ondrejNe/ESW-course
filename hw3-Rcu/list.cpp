#include "list.hpp"
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined (USE_MUTEX)
#  define rd_lock(lock)         pthread_mutex_lock(lock)
#  define rd_unlock(lock)       pthread_mutex_unlock(lock)
#  define wr_lock(lock)         pthread_mutex_lock(lock)
#  define wr_unlock(lock)       pthread_mutex_unlock(lock)
#elif defined (USE_RWLOCK)
#  define rd_lock(lock)         pthread_rwlock_rdlock(lock)
#  define rd_unlock(lock)       pthread_rwlock_unlock(lock)
#  define wr_lock(lock)         pthread_rwlock_wrlock(lock)
#  define wr_unlock(lock)       pthread_rwlock_unlock(lock)
#elif defined (USE_RCU)
#  define rd_lock(lock)         rcu_read_lock()
#  define rd_unlock(lock)       rcu_read_unlock()
#  define wr_lock(lock) // TODO
#  define wr_unlock(lock) //TODO
#else
#  error "No lock type defined"
#endif

#ifdef DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif

unsigned calc_checksum(const char *str)
{
    unsigned sum = 0;
    while (*str)
        sum += *str++;
    return sum;
}

void esw_list_init(LIST_TYPE *list)
{
#if defined (USE_MUTEX)
    CHECK(pthread_mutex_init(&list->lock, NULL));
    list->head = NULL;
#elif defined (USE_RWLOCK)
    CHECK(pthread_rwlockattr_init(&list->attr));
    CHECK(
        pthread_rwlockattr_setkind_np(
            &list->attr,
            PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP)
        );
    CHECK(pthread_rwlock_init(&list->lock, &list->attr));
    list->head = NULL;
#elif defined (USE_RCU)
    CDS_INIT_LIST_HEAD(list);
#else
#error "No lock type defined"
#endif
}

void esw_list_push(LIST_TYPE *list, const char *const key, const char *const value)
{
    assert(list);
    assert(key);
    assert(value);

    debug_printf("Pushing %s: %s\n", key, value);

    esw_node_t *node = esw_list_create_node(key, value);
#if defined (USE_MUTEX) || defined (USE_RWLOCK)
    wr_lock(&list->lock);
    node->next = list->head;
    list->head = node;
    wr_unlock(&list->lock);
#elif defined (USE_RCU)
    cds_list_add_tail_rcu(&node->node, list);
#endif
}

#if defined (USE_RCU)
void free_node_rcu(struct rcu_head *head)
{
	struct esw_node *node = caa_container_of(head, struct esw_node, rcu_head);

	esw_list_free_node(node);
}
#endif

void esw_list_update(LIST_TYPE *list, const char *const key, const char *const value)
{
    assert(list);

    /* Replaces first occurrence in the list */
#if defined (USE_MUTEX) || defined (USE_RWLOCK)
    wr_lock(&list->lock);
    esw_node_t *current = list->head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            char *new_value = strdup(value);
            debug_printf("Updating %s: %s -> %s\n", key, current->value, new_value);
            free(current->value);
            current->value = new_value;
            current->checksum = calc_checksum(new_value);
            break;
        }
        current = current->next;
    }
    wr_unlock(&list->lock);
#elif defined (USE_RCU)
    struct esw_node *current, *temp;
    cds_list_for_each_entry_safe(current, temp, list, node) {
		if (strcmp(current->key, key) == 0) {
            struct esw_node* new_node = esw_list_create_node(key, value);
            cds_list_replace_rcu(&current->node, &new_node->node);
            call_rcu(&current->rcu_head, free_node_rcu);
            break;
        }
	}
#endif
}

bool esw_list_find(LIST_TYPE *list, const char *const key, char *value, const size_t max_len)
{
    bool found = false;
    assert(list);
    assert(key);

    rd_lock(&list->lock);
#if defined (USE_MUTEX) || defined (USE_RWLOCK)
    esw_node_t *current = list->head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (strlen(current->value) < max_len) {
                strcpy(value, current->value);
                if (calc_checksum(value) != current->checksum)
                    errx(1, "%s:%d wrong checksum", __FILE__, __LINE__);
            } else {
                strncpy(value, current->value, max_len - 1);
                value[max_len - 1] = '\0';
                if (calc_checksum(value) != current->checksum)
                    errx(1, "%s:%d wrong checksum", __FILE__, __LINE__);
            }
            found = true;
            break;
        }
        current = current->next;
    }
#elif defined (USE_RCU)
    struct esw_node *current;
    cds_list_for_each_entry_rcu(current, list, node) {
		if (strcmp(current->key, key) == 0) {
            if (strlen(current->value) < max_len) {
                strcpy(value, current->value);
                if(calc_checksum(value) != current->checksum) err(1, "%s:%d wrong checksum", __FILE__, __LINE__);
            } else {
                strncpy(value, current->value, max_len - 1);
                value[max_len - 1] = '\0';
                if(calc_checksum(value) != current->checksum) err(1, "%s:%d wrong checksum", __FILE__, __LINE__);
            }
            found = true;
            break;
        }
	}
#endif
    rd_unlock(&list->lock);

    return found;
}

esw_node_t *esw_list_create_node(const char *const key, const char *const value)
{
    esw_node_t *node = (esw_node_t *)calloc(1, sizeof(esw_node_t));
    node->key = strdup(key);
    node->value = strdup(value);
    node->checksum = calc_checksum(value);

    return node;
}

void esw_list_free_node(esw_node_t *node)
{
    free(node->key);
    free(node->value);
    free(node);
}

void esw_list_free_content(LIST_TYPE *list)
{
#if defined (USE_MUTEX) || defined (USE_RWLOCK)
    esw_node_t *current;
    esw_node_t *tmp;
    assert(list != NULL);
    current = list->head;
    while (current) {
        tmp = current;
        current = current->next;
        esw_list_free_node(tmp);
    }
#elif defined (USE_RCU)
    // TODO (not necessary)
#endif
}

void esw_list_free(LIST_TYPE *list)
{
    esw_list_free_content(list);
    free(list);
}
