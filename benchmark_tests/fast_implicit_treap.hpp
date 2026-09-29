#ifndef FAST_IMPLICIT_TREAP_HPP
#define FAST_IMPLICIT_TREAP_HPP

#include <iostream>
#include <vector>

using namespace std;
typedef long long ll;


struct Node {
    int prior, sz, l, r;
    ll val, sum, lazy;

    Node() : prior(0), sz(0), l(0), r(0), val(0), sum(0), lazy(0) {}
    Node(ll v, int p) : val(v), sum(v), lazy(0), prior(p), sz(1), l(0), r(0) {}
};

struct ImplicitTreap {

    vector<Node> t;
    int root;
    unsigned int seed;

    inline int rnd() {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;

        return seed & 0x7fffffff; 
    }

    ImplicitTreap(int max_nodes) {

        // We Can Also Use Random Seed
        seed = 123456789;
        root = 0;
        t.reserve(max_nodes + 1);
        t.push_back(Node(0, 0)); 
        t[0].sz = 0; 
    }

    inline int new_node(ll v) {
        t.push_back(Node(v, rnd()));
        return t.size() - 1;
    }

    inline int sz(int v) { return v ? t[v].sz : 0; }
    inline ll sum(int v) { return v ? t[v].sum : 0; }

    inline void apply(int v, ll lazy_val) {
        if (!v) return;
        t[v].val += lazy_val;
        t[v].sum += lazy_val * t[v].sz;
        t[v].lazy += lazy_val;
    }

    inline void push(int v) {
        if (v && t[v].lazy != 0) {
            apply(t[v].l, t[v].lazy);
            apply(t[v].r, t[v].lazy);
            t[v].lazy = 0;
        }
    }

    inline void pull(int v) {
        if (v) {
            t[v].sz = 1 + sz(t[v].l) + sz(t[v].r);
            t[v].sum = t[v].val + sum(t[v].l) + sum(t[v].r);
        }
    }

    void split(int v, int k, int &l, int &r) {
        if (!v) { l = r = 0; return; }
        push(v);
        int left_sz = sz(t[v].l);
        if (k <= left_sz) {
            split(t[v].l, k, l, t[v].l);
            r = v;
        } else {
            split(t[v].r, k - left_sz - 1, t[v].r, r);
            l = v;
        }
        pull(v);
    }

    void merge(int &v, int l, int r) {
        if (!l || !r) { v = l ? l : r; return; }
        push(l); push(r);
        if (t[l].prior > t[r].prior) {
            merge(t[l].r, t[l].r, r);
            v = l;
        } else {
            merge(t[r].l, l, t[r].l);
            v = r;
        }
        pull(v);
    }

    void insert(int pos, ll val) {
        int t1, t2;
        split(root, pos, t1, t2);
        int node = new_node(val);
        merge(t1, t1, node);
        merge(root, t1, t2);
    }

    void erase(int pos) {
        int t1, t2, t3;
        split(root, pos + 1, t1, t3);
        split(t1, pos, t1, t2);
        merge(root, t1, t3);
    }

    void update_range(int L, int R, ll val) {
        if (L > R) return;
        int t1, t2, t3;
        split(root, R + 1, t1, t3);
        split(t1, L, t1, t2);
        apply(t2, val);
        merge(t1, t1, t2);
        merge(root, t1, t3);
    }

    ll query_range(int L, int R) {
        if (L > R) return 0;
        int t1, t2, t3;
        split(root, R + 1, t1, t3);
        split(t1, L, t1, t2);
        ll ans = sum(t2);
        merge(t1, t1, t2);
        merge(root, t1, t3);
        return ans;
    }
};

#endif
