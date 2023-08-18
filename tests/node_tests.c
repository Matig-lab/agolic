#include "../src/node.h"
#include <criterion/criterion.h>

#define SAMPLE_DATA 10
#define SAMPLE_DATA2 20
#define SAMPLE_DATA3 30
#define SAMPLE_NODE_INSERTS 10000

Test(node, memory_management) {
    Node *node = node_alloc(SAMPLE_DATA);
    cr_assert(node != NULL, "node_alloc() returned NULL");
    cr_assert_eq(node->data, SAMPLE_DATA,
                 "Node data with value %d, expected %d", node->data,
                 SAMPLE_DATA);
    node_destroy_all(&node);
    cr_assert_null(node, "node_destroy_all() should make node NULL");

    node = node_alloc(SAMPLE_DATA2);
    cr_expect(node != NULL,
              "node_alloc() after node_destroy_all() returned NULL");
    cr_expect(node->data == SAMPLE_DATA2,
              "Node data (after node_destroy()) with value %d, expected %d",
              node->data, SAMPLE_DATA2);
    node_destroy_all(&node);

    node = node_alloc(SAMPLE_DATA);
    node->next = node_alloc(SAMPLE_DATA);

    node_destroy_all(&node);
    cr_assert_null(node, "node_destroy_all() should make node NULL");
}

Test(node, node_len) {
    Node *node = node_alloc(SAMPLE_DATA);
    int len = node_len(node);
    cr_expect(len == 1, "node_len() should return 1 instead of %d", len);
    node_destroy(&node);

    node = node_alloc(SAMPLE_DATA);
    node->next = node_alloc(SAMPLE_DATA2);
    node->next->next = node_alloc(SAMPLE_DATA2);
    len = node_len(node);
    cr_assert(len == 3, "node_len() should return 3 instead of %d", len);
    node_destroy_all(&node);
}

Test(node, node_pop) {
    Node *list = node_alloc(SAMPLE_DATA);
    Node *popped = NULL;

    for (int i = 0; i < 5; i++) {
        node_append_uniq(&list, SAMPLE_DATA2);
    }
    int len = node_len(list);
    popped = node_pop(&list);
    int len_after_pop = node_len(list);
    cr_assert_eq(len - 1, len_after_pop,
                 "List after node_pop should have 1 node less");
    cr_assert_not_null(popped, "Popped node should not be NULL");
    cr_assert_eq(popped->data, SAMPLE_DATA,
                 "Popped node-> should be %d instead of %d", SAMPLE_DATA,
                 popped->data);
    node_destroy_all(&list);
    node_destroy_all(&popped);
    cr_assert_null(list);
    cr_assert_null(popped);
}

Test(node, node_append) {
    Node *node = NULL;
    int len = 0;

    for (int i = 0; i < SAMPLE_NODE_INSERTS; i++) {
        node_append(&node, SAMPLE_DATA);
    }
    len = node_len(node);
    cr_assert_eq(len, SAMPLE_NODE_INSERTS,
                 "node_append() should append %d instead of %d",
                 SAMPLE_NODE_INSERTS, len);
    node_destroy_all(&node);

    Node *new_node = node_alloc(SAMPLE_DATA);
    node_append_node(&node, new_node);

    cr_assert_eq(node, new_node);
    cr_assert_eq(node->data, SAMPLE_DATA);
    cr_assert_null(node->next);

    node_destroy_all(&node);
    Node *new_node1 = node_alloc(SAMPLE_DATA);
    Node *new_node2 = node_alloc(SAMPLE_DATA2);

    node_append_node(&node, new_node1);
    node_append_node(&node, new_node2);

    cr_assert_eq(node, new_node1);
    cr_assert_eq(node->data, SAMPLE_DATA);

    cr_assert_not_null(node->next);
    cr_assert_eq(node->next, new_node2);
    cr_assert_eq(node->next->data, SAMPLE_DATA2);
    cr_assert_null(node->next->next);

    node_destroy_all(&node);

    for (int i = 0; i < SAMPLE_NODE_INSERTS; i++) {
        node_append_uniq(&node, i);
    }
    len = node_len(node);
    cr_assert_eq(len, SAMPLE_NODE_INSERTS,
                 "node_append_uniq() should append %d instead of %d",
                 SAMPLE_NODE_INSERTS, len);
    node_destroy_all(&node);

    node_append_uniq(&node, SAMPLE_DATA);
    node_append_uniq(&node, SAMPLE_DATA);
    node_append_uniq(&node, SAMPLE_DATA2);
    node_append_uniq(&node, SAMPLE_DATA2);
    node_append_uniq(&node, SAMPLE_DATA3);
    node_append_uniq(&node, SAMPLE_DATA3);
    len = node_len(node);
    cr_assert_eq(len, 3,
                 "node_append_uniq() should add 3 unique nodes instead of %d",
                 len);
    node_destroy_all(&node);
}

Test(node, node_concat) {
    Node *list1 = NULL;
    node_append_uniq(&list1, SAMPLE_DATA);
    Node *list2 = NULL;
    node_append_uniq(&list2, SAMPLE_DATA2);
    node_append_uniq(&list2, SAMPLE_DATA3);
    node_concat(&list1, &list2);
    cr_assert_eq(node_len(list1), 3);
    cr_assert_eq(node_len(list2), 0);
    cr_assert_null(list2);
    node_destroy_all(&list1);
    node_destroy_all(&list2);

    node_append_uniq(&list2, SAMPLE_DATA);
    node_append_uniq(&list2, SAMPLE_DATA2);
    cr_assert_not_null(list2);
    node_concat(&list1, &list2);
    int len = node_len(list1);
    cr_assert_eq(len, 2, "List 1 should have a length of 2 instead of %d", len);
    node_destroy_all(&list1);
    node_destroy_all(&list2);
}

Test(node, node_insert_head) {
    Node *head = NULL;
    node_insert_head(&head, SAMPLE_DATA);
    cr_assert_eq(head->data, SAMPLE_DATA);
    node_destroy_all(&head);

    node_insert_head(&head, SAMPLE_DATA);
    node_insert_head(&head, SAMPLE_DATA2);
    cr_assert_eq(head->data, SAMPLE_DATA2);
    node_destroy_all(&head);
}

Test(node, node_insert_head_node) {
    Node *head = NULL;
    Node *node = node_alloc(SAMPLE_DATA);
    node_insert_head_node(&head, node);
    cr_assert_eq(head->data, SAMPLE_DATA);
    node_destroy_all(&head);

    head = node_alloc(SAMPLE_DATA);
    node = node_alloc(SAMPLE_DATA2);

    node_insert_head_node(&head, node);
    cr_assert_eq(head->data, SAMPLE_DATA2);
    node_destroy_all(&head);
}
