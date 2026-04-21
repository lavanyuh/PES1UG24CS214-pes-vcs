#include "tree.h"
#include <stdio.h>

// forward declarations
int tree_from_index(ObjectID *id_out);
int commit_create(const ObjectID *tree_id, const char *message, ObjectID *commit_id);

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