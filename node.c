#include "node.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

Node *node_alloc(int data) {
    Node *new_node = malloc(sizeof(*new_node));
    new_node->next = NULL;
    new_node->data = data;
    return new_node;
}

void node_destroy(Node **node) {
    if ((*node)->next != NULL)
        return;
    free(*node);
    *node = NULL;
}

int node_len(Node *head) {
    Node *current = head;
    int len = 0;
    while (current) {
        len++;
        current = current->next;
    }
    return len;
}

void node_append_uniq(Node **head, int data) {
    if (!*head) {
        Node *new_node = node_alloc(data);
        *head = new_node;
        return;
    }

    Node *current = *head;
    while (current->next) {
        if (current->data == data) {
            return;
        }
        current = current->next;
    }
    Node *new_node = node_alloc(data);
    current->next = new_node;
}

void node_insert_head(Node **head, int data) {
    assert(false && "Do not use node_insert_head, must be reviewed");
    Node *new_node = node_alloc(data);
    if (!*head) {
        *head = new_node;
        return;
    }

    new_node->next = (*head);
    *head = new_node;
}

void node_concat(Node *head, Node **tail) {
    if (!tail)
        return;
    if (!head) {
        head = *tail;
        return;
    }

    Node *current = head;
    while (current->next) {
        current = current->next;
    }
    current->next = *tail;
    *tail = NULL;
}

void node_delete_by_index(Node **head, int index) {
    if (!*head || index < 0)
        return;

    int i = 0;
    Node *current = *head;
    Node *last = current;

    if (i == 0) {
        *head = current->next;
        node_destroy(&current);
        return;
    }

    while (current->next) {
        if (i == index) {
            last->next = current->next;
            node_destroy(&current);
            break;
        }
        last = current;
        current = current->next;
    }
}

void node_delete_by_data(Node **head, int data) {
    if (!*head)
        return;

    if ((*head)->data == data) {
        Node *temp = *head;
        *head = (*head)->next;
        node_destroy(&temp);
        return;
    }

    Node *current = *head;
    Node *last = current;
    while (current && current->data != data) {
        last = current;
        current = current->next;
    }
    last->next = current->next;
    node_destroy(&current);
}

void node_destroy_all(Node **head) {
    if (!*head)
        return;

    Node *current = *head;
    while (current) {
        Node *tmp = current;
        current = current->next;
        node_destroy(&tmp);
    }
    *head = NULL;
}
