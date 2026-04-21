#include "tree.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// forward declaration
int object_write(ObjectType type, const void *data, size_t len, ObjectID *id_out);

int main() {
    Tree tree;
    tree.count = 1;

    tree.entries[0].mode = 0100644;

    strcpy(tree.entries[0].name, "file1.txt");

    memset(tree.entries[0].hash.hash, 'a', HASH_SIZE);

    void *data;
    size_t len;

    if (tree_serialize(&tree, &data, &len) != 0) {
        printf("serialize failed\n");
        return 1;
    }

    ObjectID id;
    if (object_write(OBJ_TREE, data, len, &id) != 0) {
        printf("write failed\n");
        return 1;
    }

    printf("Tree object created successfully\n");

    free(data);
    return 0;
}