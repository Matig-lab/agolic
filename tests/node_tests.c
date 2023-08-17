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
    node_destroy(&node);
    cr_assert_null(node, "node_destroy() should make node NULL");

    node = node_alloc(SAMPLE_DATA2);
    cr_expect(node != NULL, "node_alloc() after node_destroy() returned NULL");
    cr_expect(node->data == SAMPLE_DATA2,
              "Node data (after node_destroy()) with value %d, expected %d",
              node->data, SAMPLE_DATA2);
    node_destroy(&node);

    node = node_alloc(SAMPLE_DATA);
    node->next = node_alloc(SAMPLE_DATA);

    node_destroy(&node);
    cr_assert_not_null(node,
                       "node_destroy() should not destroy node with childs");

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

Test(node, node_append_uniq) {
    Node *node = NULL;

    for (int i = 0; i < SAMPLE_NODE_INSERTS; i++) {
        node_append_uniq(&node, i);
    }
    int len = node_len(node);
    cr_assert_eq(len, SAMPLE_NODE_INSERTS,
                 "node_append() should append %d instead of %d",
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
    node_concat(list1, &list2);
    cr_assert_eq(node_len(list1), 3);
    cr_assert_eq(node_len(list2), 0);
    node_destroy_all(&list1);
    node_destroy_all(&list2);
}
