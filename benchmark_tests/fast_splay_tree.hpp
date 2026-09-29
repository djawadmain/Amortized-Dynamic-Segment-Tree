#ifndef FAST_SPLAY_TREE_HPP
#define FAST_SPLAY_TREE_HPP

#include<iostream>
#include<vector>

struct SplayTree {
    
    struct Node {
        int ch[2], p, sz;
        long long val, sum, lazy;
    };

    std::vector<Node> tr;
    int root;

    inline int new_node(long long v, int p){
        tr.push_back({{0, 0}, p, 1, v, v, 0});
        return tr.size() - 1;
    }

    SplayTree(int capacity){

        tr.reserve(capacity + 5);
        tr.push_back({{0, 0}, 0, 0, 0, 0, 0});
        root = new_node(0, 0);
        int r_dummy = new_node(0, root);
        tr[root].ch[1] = r_dummy;
        push_up(root);
    }

    inline void push_up(int x) {
        if (!x) return;
        tr[x].sz = tr[tr[x].ch[0]].sz + tr[tr[x].ch[1]].sz + 1;
        tr[x].sum = tr[tr[x].ch[0]].sum + tr[tr[x].ch[1]].sum + tr[x].val;
    }

    inline void apply(int x, long long v) {
        if (!x) return;
        tr[x].val += v;
        tr[x].sum += v * tr[x].sz;
        tr[x].lazy += v;
    }

    inline void push_down(int x) {
        if (x && tr[x].lazy) {
            apply(tr[x].ch[0], tr[x].lazy);
            apply(tr[x].ch[1], tr[x].lazy);
            tr[x].lazy = 0;
        }
    }

    inline int get_dir(int x) {
        return tr[tr[x].p].ch[1] == x;
    }

    inline void rotate(int x) {
        int y = tr[x].p, z = tr[y].p, k = get_dir(x);
        
        if (z) tr[z].ch[get_dir(y)] = x;
        tr[x].p = z;

        tr[y].ch[k] = tr[x].ch[k ^ 1];
        if (tr[x].ch[k ^ 1]) tr[tr[x].ch[k ^ 1]].p = y;

        tr[x].ch[k ^ 1] = y;
        tr[y].p = x;

        push_up(y);
        push_up(x);
    }

    inline void splay(int x, int goal = 0) {

        if (!x) return;

        std::vector<int> path;
        for (int curr = x; curr != goal; curr = tr[curr].p) {
            path.push_back(curr);
        }

        while (!path.empty()) {
            push_down(path.back());
            path.pop_back();
        }

        while (tr[x].p != goal) {
            int y = tr[x].p, z = tr[y].p;
            if (z != goal) {
                if (get_dir(x) == get_dir(y)) rotate(y);
                else rotate(x);
            }
            rotate(x);
        }
        if (!goal) root = x;
    }

    inline int kth(int k) {
        int x = root;
        while (x) {
            push_down(x);
            int left_sz = tr[tr[x].ch[0]].sz;
            if (k <= left_sz) {
                x = tr[x].ch[0];
            } else if (k == left_sz + 1) {
                return x;
            } else {
                k -= left_sz + 1;
                x = tr[x].ch[1];
            }
        }
        return 0;
    }

    inline void insert(int pos, long long val) {
        int l_node = kth(pos);
        splay(l_node, 0);
        int r_node = kth(pos + 1);
        splay(r_node, l_node);

        int x = new_node(val, r_node);
        tr[r_node].ch[0] = x;

        push_up(r_node);
        push_up(l_node);
    }

    inline void update_range(int l, int r, long long val) {
        int l_node = kth(l);
        splay(l_node, 0);
        int r_node = kth(r + 2);
        splay(r_node, l_node);

        apply(tr[r_node].ch[0], val);
        push_up(r_node);
        push_up(l_node);
    }

    inline long long query_range(int l, int r) {
        int l_node = kth(l);
        splay(l_node, 0);
        int r_node = kth(r + 2);
        splay(r_node, l_node);

        return tr[tr[r_node].ch[0]].sum;
    }

    inline int size() {
        return tr[root].sz - 2;
    }
};

#endif
