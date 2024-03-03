#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;
    }
    list_add(&node->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;
    }
    list_add_tail(&node->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_first_entry(head, element_t, list);

    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->next);
    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_last_entry(head, element_t, list);

    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->prev);
    return node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head)
        return false;
    struct list_head **indir = &head->next;

    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next)
        indir = &(*indir)->next;
    struct list_head *del = *indir;
    list_del(del);
    q_release_element(list_entry(del, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;
    element_t *entry, *safe;
    bool dup = false;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (&safe->list != head && !strcmp(entry->value, safe->value)) {
            list_del(&entry->list);
            q_release_element(entry);
            dup = true;
        } else if (dup) {
            list_del(&entry->list);
            q_release_element(entry);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    struct list_head *head_tmp;
    for (head_tmp = head;
         head_tmp->next != head && head_tmp->next->next != head;
         head_tmp = head_tmp->next->next) {
        struct list_head *node = head_tmp->next->next;
        list_move(node, head_tmp);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head **indir = &head->next->next;
    while (*indir != head) {
        list_move(*indir, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    struct list_head **indir = &head->next->next, *sub_head = head;
    int size = q_size(head);
    while (size >= k) {
        for (int i = 0; i < k - 1; i++) {
            list_move(*indir, sub_head);
        }
        sub_head = (*indir)->prev;
        indir = &(*indir)->next;
        size -= k;
    }
}

static struct list_head *merge_two_lists(struct list_head *L,
                                         struct list_head *R)
{
    struct list_head *head = NULL;
    struct list_head **ptr = &head;

    for (; L && R; ptr = &(*ptr)->next) {
        element_t *left = list_entry(L, element_t, list);
        element_t *right = list_entry(R, element_t, list);
        if (strcmp(left->value, right->value) > 0) {
            *ptr = R;
            R = R->next;
        } else {
            *ptr = L;
            L = L->next;
        }
    }
    *ptr = (struct list_head *) ((uintptr_t) R | (uintptr_t) L);
    return head;
}

static struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *slow = head, *fast = head;

    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    struct list_head *mid = slow;
    slow->prev->next = NULL;

    struct list_head *left = merge_sort(head);
    struct list_head *right = merge_sort(mid);

    return merge_two_lists(left, right);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (list_empty(head))
        return;

    head->prev->next = NULL;
    head->next = merge_sort(head->next);
    struct list_head *prev = head, *iter = head->next;
    for (; iter; iter = iter->next) {
        iter->prev = prev;
        prev = iter;
    }
    prev->next = head;
    head->prev = prev;
    if (descend)
        q_reverse(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        for (struct list_head *node = &safe->list; node != head;
             node = node->next) {
            element_t *node_entry = list_entry(node, element_t, list);
            if (strcmp(entry->value, node_entry->value) > 0) {
                list_del(&entry->list);
                q_release_element(entry);
                break;
            }
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        for (struct list_head *node = &safe->list; node != head;
             node = node->next) {
            element_t *node_entry = list_entry(node, element_t, list);
            if (strcmp(entry->value, node_entry->value) < 0) {
                list_del(&entry->list);
                q_release_element(entry);
                break;
            }
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    int list_size = q_size(head);
    if (list_size < 2)
        return 0;

    queue_contex_t *entry;
    list_for_each_entry (entry, head, chain) {
        entry->q->prev->next = NULL;
    }

    struct list_head *merge_head, *merge_tail = head->prev;
    for (int i = 0; i < (list_size + 1) / 2; i++) {
        merge_head = head->next;
        while (merge_head->prev != merge_tail && merge_head != merge_tail) {
            struct list_head *head_q =
                list_entry(merge_head, queue_contex_t, chain)->q;
            struct list_head *tail_q =
                list_entry(merge_tail, queue_contex_t, chain)->q;
            head_q->next = merge_two_lists(head_q->next, tail_q->next);
            INIT_LIST_HEAD(tail_q);
            merge_head = merge_head->next;
            merge_tail = merge_tail->prev;
        }
    }
    struct list_head *q_head = list_entry(head->next, queue_contex_t, chain)->q;
    struct list_head *prev = q_head, *iter = q_head->next;
    for (; iter; iter = iter->next) {
        iter->prev = prev;
        prev = iter;
    }
    prev->next = q_head;
    q_head->prev = prev;
    return q_size(q_head);
}
