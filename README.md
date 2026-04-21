

**Name:** Lavanya K  
**SRN:** PES1UG24CS214  
**Section:** 4D


---


## Phase 1 — Object Storage

### Implementation

The functions `object_write` and `object_read` were implemented in `object.c`.

`object_write` constructs a header in the format `"blob <size>\0"` and appends the actual data. A SHA-256 hash is computed over this combined content. If an object with the same hash already exists, it is not rewritten. Otherwise, the object is stored inside a directory determined by the first two characters of the hash. The write is done safely using a temporary file followed by an atomic rename.

`object_read` reconstructs the file path using the hash, reads the content, verifies integrity by recomputing the hash, and then extracts the data portion after the header.

### Screenshot 1A — `./test_objects`


<img width="932" height="299" alt="1A" src="https://github.com/user-attachments/assets/4d2a2d8a-cb89-47ea-926e-52e65738b5b2" />


### Screenshot 1B — Object directory structure

<img width="936" height="287" alt="1B" src="https://github.com/user-attachments/assets/fa69fd8b-a533-4ccd-8ab8-0ff8c12ffe3b" />


---

## Phase 2 — Tree Objects

### Implementation

The function `tree_from_index` in `tree.c` builds a hierarchical tree from staged files.

Entries are sorted by path and processed recursively. Files are added directly, while directories trigger recursive subtree construction. Each subtree is written as a separate tree object and referenced by its parent.

Serialization ensures deterministic ordering, so identical directory structures always generate the same hash.

### Screenshot 2A — Tree test output

<img width="856" height="178" alt="2A" src="https://github.com/user-attachments/assets/84aab331-96e5-440b-8b26-552dd08f8165" />


### Screenshot 2B — Raw tree object (xxd)

<img width="926" height="337" alt="2B" src="https://github.com/user-attachments/assets/b1aacda2-e1c3-4836-8f89-b6f30b34b640" />



---

## Phase 3 — Index / Staging Area

### Implementation

The staging area is handled using `index_load`, `index_save`, and `index_add`.

- `index_load` reads `.pes/index` and reconstructs entries. If the file does not exist, it initializes an empty index.
- `index_save` writes entries to a temporary file and replaces the original atomically.
- `index_add` reads file contents, stores them as blobs, and updates the index entry.

### Screenshot 3A — `pes init`, `pes add`, `pes status`

<img width="664" height="558" alt="3A" src="https://github.com/user-attachments/assets/19d17204-8441-4f21-8a39-386ac742d439" />


### Screenshot 3B — Index contents

<img width="928" height="180" alt="3B" src="https://github.com/user-attachments/assets/e7f8ebf9-86c9-47e0-8d8a-fa8b357e0619" />


---

## Phase 4 — Commits and History

### Implementation

The function `commit_create` generates commits.

It first creates a tree snapshot from the index. Then a commit structure is populated with:
- tree hash  
- timestamp  
- author (from `PES_AUTHOR` environment variable)  
- commit message  

If a previous commit exists, it is stored as the parent. The commit is serialized and written to the object store. Finally, the current branch reference is updated to point to the new commit.

### Screenshot 4A — `./pes log`

<img width="948" height="587" alt="4A" src="https://github.com/user-attachments/assets/175080ce-4551-487f-8b04-1ce57084247e" />


### Screenshot 4B — Object store growth

```bash
find .pes -type f | sort
```

<img width="957" height="559" alt="4B" src="https://github.com/user-attachments/assets/e7d32357-1f34-46b6-9038-457a5d905a64" />


### Screenshot 4C — Reference chain

```bash
cat .pes/refs/heads/main
cat .pes/HEAD
```

<img width="959" height="148" alt="4C" src="https://github.com/user-attachments/assets/a730bf15-3179-461d-bfe8-18f8fda33bf3" />


---

## Integration Test

![Integration Test](screenshots/integration.png)

---

## Phase 5 — Branching and Checkout (Analysis)

### Q5.1 — Checkout Implementation

To implement `pes checkout <branch>`, the HEAD file must be updated to point to the selected branch. The working directory must then be updated to match the tree snapshot of that branch.

This involves:
- Reading the current and target commit trees  
- Identifying files to create, delete, or update  
- Applying changes recursively  

A major challenge is handling uncommitted changes. If a file has local modifications and would be overwritten, the operation must be aborted to prevent data loss.

---

### Q5.2 — Detecting Dirty Working Directory

Conflicts can be detected using the index and object store:

1. Compare working directory files with index metadata  
2. Identify files that differ between current and target commits  
3. If a file appears in both sets, it is a conflict  

If conflicts exist, checkout must stop and notify the user.

---

### Q5.3 — Detached HEAD

In detached HEAD state, HEAD points directly to a commit instead of a branch.

New commits created in this state are not referenced by any branch and can become unreachable. To recover, a branch must be created pointing to the current commit.

---

## Phase 6 — Garbage Collection (Analysis)

### Q6.1 — Garbage Collection Algorithm

Garbage collection follows a mark-and-sweep approach:

- **Mark phase:** Traverse all reachable commits, trees, and blobs starting from branch references  
- **Sweep phase:** Delete objects not present in the reachable set  

A hash set is used to track reachable objects efficiently.

---

### Q6.2 — GC Race Condition

Running GC alongside commits can cause data loss. If a new object is created after the mark phase but before sweep, it may be incorrectly deleted.

To prevent this, a grace period is used — recently created objects are not removed. Git also avoids running GC concurrently with active operations.

---

