#ifndef _NODE_H_
#define _NODE_H_

typedef struct Node Node;
struct Node {
    int data;
    Node *next;
};

Node *node_alloc(int data);
void node_destroy(Node **node);
int node_len(Node *head);
void node_append(Node **head, int data);
void node_append_node(Node **head, Node *node);
void node_append_uniq(Node **head, int data);
void node_insert_head(Node **head, int data);
void node_concat(Node **head, Node **tail);
Node *node_pop(Node **head);
void node_delete_by_index(Node **head, int index);
void node_delete_by_data(Node **head, int data);
void node_destroy_all(Node **head);

#endif // _NODE_H_
