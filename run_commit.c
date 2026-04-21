#include "tree.h"
#include <stdio.h>

// forward declarations
int tree_from_index(ObjectID *id_out);
int commit_create(const ObjectID *tree_id, const char *message, ObjectID *commit_id) {

    char buffer[1024];

    char tree_hex[HASH_SIZE * 2 + 1];
    for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(tree_hex + 2*i, "%02x", tree_id->hash[i]);
    }

    int len = snprintf(buffer, sizeof(buffer),
                       "tree %s\nmessage %s\n",
                       tree_hex, message);

    printf("DEBUG: commit content:\n%s\n", buffer);

    if (len < 0) {
        printf("DEBUG: snprintf failed\n");
        return -1;
    }

    int res = object_write(OBJ_COMMIT, buffer, len, commit_id);

    printf("DEBUG: object_write returned %d\n", res);

    return res;
}

int main() {
    ObjectID tree_id;
    ObjectID commit_id;

    if (tree_from_index(&tree_id) != 0) {
        printf("tree failed\n");
        return 1;
    }

    if (commit_create(&tree_id, "my first commit", &commit_id) != 0) {
        printf("commit failed\n");
        return 1;
    }

    printf("Commit created successfully\n");
    return 0;
}