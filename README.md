## The Origin & The Benchmarks: A Hardware-Aware Approach

A few months ago, I independently conceptualized an idea for a dynamic data structure. However, it wasn't until recently that I finally found the time to sit down and write a truly optimized, production-ready implementation for it. 

Curious to see if anyone else had thought of this, I dug deep into the Codeforces archives and stumbled upon a heavily forgotten [blog post from 12 years ago](http://codeforces.com/blog/entry/12285). The author successfully implemented the core base-logic: traversing down to a leaf and splitting it into two new children. 

**However, there was a major bottleneck in that ancient version:** It relied on **AVL Tree Rotations** (`left_rotate`, `right_rotate`) to keep the height balanced. In modern competitive programming, heavy pointer-chasing and branch-heavy rotation conditions destroy Cache Locality and inflate the constant factor massively.

I realized that instead of doing local AVL rotations, we could completely ditch tree rotations and use an **Amortized Subtree Rebuilding** approach (conceptually similar to Scapegoat Trees). This eliminates branch-heavy balancing logic and, more importantly, allows us to physically defragment the tree in memory during rebuilds, achieving absolute maximum cache-friendliness.

Once I finished my implementation, I decided to benchmark it against highly optimized, memory-leak-free versions of **Implicit Treap** and **Splay Tree**. Frankly, I didn't expect much at first, but seeing the great results is exactly what motivated me to push this forward and share it with the community.

---

### System Checker Data & Compile Method

> **Hardware:** i7-13620H, 32GB RAM, NVMe SSD, RTX 4060  
> **Compiler Flags:** `g++ -O3 name.cpp -o name`

---

### The Benchmarks

#### Test 1: Random Operations (Random N Insertion + N Update + N Get)
*The ultimate test for branch prediction and cache misses.*

| N = | 200000 | 500000 | 1000000 | 2000000 |
| :---: | :---: | :---: | :---: | :---: |
| **Amortized Dynamic Segment Tree** | **250ms** | **1050ms** | **2900ms** | **7900ms** |
| **Implicit Treap** | 470ms | 1580ms | 4600ms | 12100ms |
| **Splay Tree** | 520ms | 1700ms | 4900ms | 13200ms |

#### Test 2: The Splay Illusion (N Push Front + Random N Update + N Get)
*Designed specifically to test the absolute best-case scenario for Splay Trees.*

| N = | 200000 | 500000 | 1000000 | 2000000 |
| :---: | :---: | :---: | :---: | :---: |
| **Amortized Dynamic Segment Tree** | **230ms** | **790ms** | **2200ms** | **5500ms** |
| **Splay Tree** | 330ms | 1020ms | 2750ms | 7500ms |
| **Implicit Treap** | 360ms | 1050ms | 2800ms | 7750ms |

#### Test 3: Heavy Range Query (Random N Insertion + 2N Update + 2N Get)
*Doubling the range queries to test traversal efficiency.*

| N = | 200000 | 500000 | 1000000 | 2000000 |
| :---: | :---: | :---: | :---: | :---: |
| **Amortized Dynamic Segment Tree** | **420ms** | **1680ms** | **5050ms** | **13900ms** |
| **Implicit Treap** | 830ms | 2820ms | 8550ms | 22100ms |
| **Splay Tree** | 880ms | 2950ms | 8650ms | 23500ms |

---

### What Are We Looking At?

These numbers tell a fascinating story. This isn't just an algorithmic improvement; it's a victory of **hardware-aware, cache-friendly architecture** over traditional pointer-chasing. 

*   **The Cache Dominance:** Even in the second test (N Push Fronts)—which is theoretically the Splay Tree's absolute home ground—our array-based structure outperforms it by a wide margin. Modern CPUs heavily favor linear memory access (Pre-allocation) over $O(1)$ pointer rotations.
*   **Non-Destructive Queries:** The performance gap becomes a chasm in the third test (Heavy Range Queries). In a Treap, a range query is a "destructive" operation requiring multiple `Split` and `Merge` calls. In our structure, range queries are completely static and non-destructive—acting exactly like a standard Segment Tree—allowing it to traverse the data at high speed without moving a single pointer.

### A Respectful Disclaimer

To be completely fair to the classics, let me be clear: **This data structure is not a universal replacement for Treap or Splay Tree.** 

Those classic pointer-based structures remain the undisputed kings when it comes to complex topological transformations, such as reversing a subsegment (`reverse(l, r)`), cyclic shifts, or cutting and pasting segments of an array. Our structure cannot natively handle those specific operations.

However, if your problem only requires dynamic insertions, and heavy range queries/updates, these benchmarks prove that you no longer need to pay the massive constant-factor penalty of pointer-based trees. In this specific domain, this implementation operates in a league of its own.

### Maintaining a Dynamic Segment Tree with Arbitrary Insertions

Suppose you need a dynamic segment tree that allows inserting elements at any arbitrary index, independent of the execution time, while keeping the tree structure intact. 

The standard approach for inserting at a specific index is to traverse down to see which child the index belongs to. Once we reach a leaf (a node of size 1), we add two new nodes: one representing the existing element, and the other representing the newly inserted element. We then replace the current leaf by merging these two new children, and the tree updates normally on the way back up.

#### The Skewed Tree Problem

While this logic is straightforward, the execution time heavily depends on the tree's height. If we naively insert elements (for example, repeatedly inserting at the very end of the array), the tree will degenerate into a skewed structure, much like a linked list. The height of the tree will easily reach $O(N)$, ruining our time complexity.

To resolve this, we can keep our insertion algorithm exactly as it is, but introduce **periodic rebuilding** for specific nodes. 

Rebuilding a node means extracting all elements within its subtree and reconstructing a perfectly balanced static segment tree from scratch, where the children are evenly halved at each step. If a node has a size of $X$, rebuilding it takes $O(X)$ time and guarantees its new height will be exactly $\lceil \log_2 X \rceil$.

Let's explore how we can use this $O(X)$ rebuild operation to our advantage.

#### Idea: Power-of-Two Rebuilds

Instead of global rebuilds, we can apply a simple rule to rebuild specific subtrees dynamically:

> **Rule:** Whenever the size of a subtree (after insertions) becomes exactly $X = 2^k$ (where $k \ge 2$), rebuild that specific subtree.

We can prove that this simple condition is sufficient to achieve an amortized $O(N \log N)$ complexity and the tree height is also bounded by $O(\log N)$

This height condition allows us to solve standard segment tree queries in $O(\log N)$.

## The Math: Why Does This Work? 

At first glance, destroying and rebuilding entire subtrees sounds like a recipe for a Time Limit Exceeded (TLE) verdict. However, the underlying mathematics guarantees a strict Amortized $O(\log N)$ time complexity. 

The proof of efficiency relies on two core pillars: bounding the maximum height of the tree, and amortizing the reconstruction cost.

### 1. The 3:1 Ratio (Bounding the Tree Height)
Ignoring the rebuild cost for a moment, how do we know the tree doesn't degenerate into a linked list (height $O(N)$)? 

The trigger for a `ReBuild` is strictly tied to powers of 2: `(sz & (sz - 1)) == 0`. 
When a subtree is rebuilt at size $S = 2^k$, it becomes a **perfect binary tree**. At this exact moment, both of its children have a size of exactly $2^{k-1}$. 

This subtree will not be rebuilt again until its total size reaches $2^{k+1}$ (which requires $2^k$ new insertions). In the absolute worst-case scenario, all $2^k$ new elements are inserted into just *one* of the children. 
- The maximum size of the heavy child becomes: $2^{k-1} + 2^k = 3 \cdot 2^{k-1}$
- The size of the light child remains: $2^{k-1}$

This guarantees that the size ratio between any two siblings will never exceed **$3:1$**. Because a fraction of the subtree (at least $1/4$) is always dropped as we move down a level, the maximum depth of the tree is strictly bounded by $O(\log_{4/3} N)$, which simplifies to $O(\log N)$. All standard `Get` and `Update` operations traverse this strictly logarithmic height.

### 2. The Amortized Rebuild Cost

**Why is the rebuilding process so efficient?**

To understand why the `ReBuild` function doesn't cause a Time Limit Exceeded (TLE), let's analyze its amortized cost. 

Suppose we trigger a rebuild operation on a node when its size reaches $sz = 2^k$. Because of our condition, we know that at some point in the past, this exact node was a perfectly balanced subtree with a size of $sz = 2^{k-1}$.

This implies that since the last time this node was perfect, exactly $2^{k-1}$ `Insert` operations have been performed inside its subtree. The time complexity to entirely rebuild this subtree of size $2^k$ is $O(2^k)$.

To find the amortized cost, we can distribute this $O(2^k)$ rebuild cost across the $2^{k-1}$ newly inserted elements that led to this rebuild. Since $O(2^k) = O(2 \cdot 2^{k-1})$, the cost distributed to each individual insertion is:

$$\frac{O(2^k)}{2^{k-1}} = O(1)$$

This means every time we insert an element, it effectively "pays" an $O(1)$ amortized cost towards the future rebuild of this specific node.

**Total Time Complexity:**

In our Segment Tree, the maximum depth is bounded by $O(\log n)$. Therefore, any single inserted element contributes to the size of at most $O(\log n)$ ancestor nodes. 

Since each element pays an $O(1)$ amortized rebuild cost for each of its ancestors, the total number of operations contributed by $n$ insertions across the entire tree is:

$$n \times O(\log n) = O(n \log n)$$

Thus, the overall time complexity for all rebuilding operations combined is strictly bounded by $O(n \log n)$, making it incredibly fast and well within the standard time limits!

---

## The Black Box: Reference Implementation

One of the biggest flaws of structures like Treap or Splay Tree is how tightly coupled their structural logic (`Split`, `Merge`, `Rotate`) is with the problem's mathematical logic. A small typo in tracking subtree sizes during a rotation can destroy the entire tree.

I designed this implementation to be a complete **Black Box**. The structural logic (Amortized Rebuilding, Memory Allocation, Pointer Management) is entirely isolated from the data logic. 

To adapt this template for *any* problem, you only need to modify three things, exactly as you would in a standard static Segment Tree:

1. `struct S` (Your node variables)
2. `Merge()` (How to combine two children)
3. `Relax()` (How to push lazy tags)
4. `Initialization logic in the insert functions` (If you want to change leafs structure)

### The Code & Explanation

Let's break down the implementation of our Segment Tree step by step.

#### 1. The Node Structure

First, we define our node structure. The variables `lc` (left child), `rc` (right child), and `sz` (subtree size) are essential for the core logic of our tree, especially since the structure changes dynamically.

```cpp
// Define your Node structure here
struct S {
    int lc, rc, sz;
    ll sum, lazy;
};
```

#### 2. Tree Initialization

Next, we define the main structure. We use `nc` to keep track of the maximum used index in our node vector (`seg`). If `nc = -1`, it means the tree is currently empty. We will discuss the `fr` variable later.

```cpp
struct SegTree {
    int fr, nc = -1;

    vector<S> seg;
    vector<int> lid, pid;

    void Add_More_Space(int n){
        seg.resize((n << 1) - 1);
        lid.resize(n);
        pid.resize(n - 1);
    }

    int get_length(){
        if (nc == -1) return 0;
        return seg[0].sz;
    }

    // n = maximum amount of insert queries
    SegTree (int n){
        Add_More_Space(n);
    }
}
```

#### 3. Core Node Operations

Here we have the standard `Merge` and `Relax` functions for lazy propagation. The `Build` function is specifically used during the insertion phase to initialize a new leaf node.

```cpp
    // ---------------------------------------------------------
    // MODIFY THESE THREE FUNCTIONS BASED ON YOUR PROBLEM
    // ---------------------------------------------------------
    void Merge(int id) {
        int lc = seg[id].lc, rc = seg[id].rc;
        seg[id].sz = seg[lc].sz + seg[rc].sz;
        seg[id].sum = seg[lc].sum + seg[rc].sum;
        seg[id].lazy = 0;
    }

    void Relax(int id) {
        int lc = seg[id].lc, rc = seg[id].rc;
        
        seg[lc].lazy += seg[id].lazy;
        seg[lc].sum += seg[id].lazy * seg[lc].sz;

        seg[rc].lazy += seg[id].lazy;
        seg[rc].sum += seg[id].lazy * seg[rc].sz;
        seg[id].lazy = 0;
    }

    void Build(int id, int x) {
        seg[id].sz = 1;
        seg[id].sum = x;
        seg[id].lazy = x;
    }
```

#### 4. The Rebuild Process (Maintaining Balance)

You can treat these three functions as a black box, but here is how they work behind the scenes to keep the tree perfectly balanced without wasting memory:


*   **`Found_Ids`**: Traverses the subtree and collects the indices of all leaves in sorted order into the `lid` vector, and the internal nodes into the `pid` vector. 

*   **Memory Efficiency**: By collecting existing IDs, we reuse allocated memory instead of building entirely new nodes.

*   **`Set_Ids`**: Uses the collected `pid` and `lid` arrays to construct a perfectly balanced Segment Tree.


```cpp
    void Found_Ids(int id, int &pt1, int &pt2) {
        if (seg[id].sz == 1) {
            lid[pt1++] = id;
            return;
        }
        Relax(id);
        Found_Ids(seg[id].lc, pt1, pt2);
        Found_Ids(seg[id].rc, pt1, pt2);
        pid[pt2++] = id;
    }

    int Set_Ids(int l, int r, int &pt1, int &pt2) {
        if (l == r) return lid[pt1++];
        int mid = (l + r) >> 1;
        int lc = Set_Ids(l, mid, pt1, pt2);
        int rc = Set_Ids(mid + 1, r, pt1, pt2);
        
        int id = pid[pt2++];
        seg[id].lc = lc, seg[id].rc = rc;
        Merge(id);
        
        return id;
    }

    void ReBuild(int id) {
        int ln = seg[id].sz;
        int pt1 = 0, pt2 = 0;
        Found_Ids(id, pt1, pt2);
        
        pt1 = 0, pt2 = 0;
        Set_Ids(0, ln - 1, pt1, pt2);
    }
```

#### 5. Dynamic Insertion

This is the core of the dynamic structure. 
`insert_index(k, x)` inserts value `x` at index `k`. If the tree is empty, it simply builds the root. Otherwise, it sets `fr = -1` and calls `Insert_DFS`. 

The variable `fr` stores the highest node (ancestor) that requires a rebuild to maintain balance. Since rebuilding an ancestor automatically fixes all its descendants, finding the highest necessary node is sufficient.

Inside `Insert_DFS`:

*   **Leaf Case:** If we reach a leaf, we split it into two nodes using unused indices (`nc + 1` and `nc + 2`). We swap siblings if their order is incorrect based on `k`, merge, and move on.

*   **Internal Node Case:** We route the query to the correct child based on subtree sizes and merge on our way up.

*   **Balance Condition:** If our subtree size is exactly a power of 2 (checked via `(seg[id].sz & (seg[id].sz - 1)) == 0`), we set `fr = id` to schedule a rebuild for this subtree.

```cpp
    void Insert_DFS(int id, int k, int x) {
        if (seg[id].sz == 1) {
            Build(nc + 1, x);
            swap(seg[nc + 2], seg[id]);
            
            seg[id].lc = nc + 1;
            seg[id].rc = nc + 2;
            if (k == 1) swap(seg[id].lc, seg[id].rc);
            
            Merge(id);
            nc += 2;
            return;
        }

        Relax(id);

        int lc = seg[id].lc, rc = seg[id].rc;
        if (seg[lc].sz >= k) Insert_DFS(lc, k, x);
        else Insert_DFS(rc, k - seg[lc].sz, x);

        Merge(id);

        if ((seg[id].sz & (seg[id].sz - 1)) == 0) fr = id;
    }

    void insert_index(int k, int x) {
        if (nc == -1) {
            Build(0, x);
            nc = 0;
            return;
        }

        fr = -1;
        Insert_DFS(0, k, x);
        if (fr != -1) ReBuild(fr);
    }
```

#### 6. Standard Segment Tree Queries

Finally, we have the standard Lazy Segment Tree operations for range updates and range queries.

```cpp
    void Update(int id, int l, int s, int e, ll x) {
        int r = l + seg[id].sz - 1;
        if (s > r or e < l) return;
        if (s <= l and e >= r) {
            seg[id].lazy += x;
            seg[id].sum += x * seg[id].sz;
            return;
        }
        Relax(id);
        
        int lc = seg[id].lc, rc = seg[id].rc;
        Update(lc, l, s, e, x);
        Update(rc, l + seg[lc].sz, s, e, x);
        
        Merge(id);
    }

    void update_range(int l, int r, ll x) {
        Update(0, 0, l, r, x);
    }

    ll Get(int id, int l, int s, int e) {
        int r = l + seg[id].sz - 1;
        if (s > r or e < l) return 0;
        if (s <= l and e >= r) return seg[id].sum;
        
        Relax(id);
        
        int lc = seg[id].lc, rc = seg[id].rc;
        return Get(lc, l, s, e) + Get(rc, l + seg[lc].sz, s, e);
    }

    ll range_sum(int l, int r) {
        return Get(0, 0, l, r);
    }
```

---

## A Note on Deletion (`Erase` Operation)

You might have noticed that I didn't include a deletion function in the implementation. If your problem strictly requires erasing elements, it is entirely possible to add it, though it comes with a trade-off.

To implement `Erase` while maintaining our amortized $O(N \log N)$ complexity, you should **not** physically remove the node right away. Instead, use a **Soft Delete** approach:

1. Introduce a new variable `rsz` (removed size) alongside `sz` in your node structure.

2. When deleting an element, just mark it as deleted, increment the `rsz` for it and all its ancestors, and ignore it during `Get` and `Update` queries.

3. Modify the rebuild condition: Whenever a subtree's total size (active elements + soft-deleted elements, meaning `sz + rsz`) reaches a power of two ($2^k$), rebuild the subtree. 

4. During this rebuild, simply drop all the soft-deleted nodes and physically reconstruct the tree using only the active elements.

While this keeps the time complexity theoretically intact, it dirties the elegant black-box code and adds a constant-factor penalty (slowing down the structure by roughly 10% to 20%). For problems that only require dynamic insertions and range queries, the current append-only version is significantly cleaner and faster.

implementations possible. 

If you have any tips or suggestions to improve them, feel free to let me know in the comments!
