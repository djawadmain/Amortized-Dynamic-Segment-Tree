#ifndef AMORTIZED_SEG_TREE_HPP
#define AMORTIZED_SEG_TREE_HPP

#include<iostream>
#include<vector>

typedef long long ll;

struct S{

    int lc, rc, sz;
    ll sum, lazy;
};

struct SegTree {

    int fr, nc = -1;

    std::vector<S> seg;
    std::vector<int> lid, pid;

    void Add_More_Space(int N){

        seg.resize((N << 1) - 1);
        lid.resize(N);
        pid.resize(N - 1);
    }

    int Get_Length(){

        if (nc == -1) return 0;
        return seg[0].sz;
    }

    // N = MAXIMUM AMOUNT OF INSERT QUERY
    SegTree (int N){

        Add_More_Space(N);
    }


    // ---------------------------------------------------------
    // MODIFY THESE THREE FUNCTIONS BASED ON YOUR PROBLEM
    // ---------------------------------------------------------

    void Merge(int id){

        int lc = seg[id].lc, rc = seg[id].rc;

        seg[id].sz = seg[lc].sz + seg[rc].sz;
        seg[id].sum = seg[lc].sum + seg[rc].sum;
        seg[id].lazy = 0;
    }

    void Relax(int id){

        int lc = seg[id].lc, rc = seg[id].rc;
        
        seg[lc].lazy += seg[id].lazy;
        seg[lc].sum += seg[id].lazy * seg[lc].sz;

        seg[rc].lazy += seg[id].lazy;
        seg[rc].sum += seg[id].lazy * seg[rc].sz;
        
        seg[id].lazy = 0;
    }

    void Build(int id, int x){

        seg[id].sz = 1;
        seg[id].sum = x;
        seg[id].lazy = x;

        return;
    }

    // ---------------------------------------------------------

    void Found_Ids(int id, int &pt1, int &pt2){

        if (seg[id].sz == 1){

            lid[pt1++] = id;
            return;
        }

        Relax(id);

        Found_Ids(seg[id].lc, pt1, pt2);
        Found_Ids(seg[id].rc, pt1, pt2);

        pid[pt2++] = id;
    }

    int Set_Ids(int l, int r, int &pt1, int &pt2){

        if (l == r) return lid[pt1++];

        int mid = (l + r) >> 1;
        
        int lc = Set_Ids(l, mid, pt1, pt2);
        int rc = Set_Ids(mid + 1, r, pt1, pt2);

        int id = pid[pt2++];

        seg[id].lc = lc, seg[id].rc = rc;
        Merge(id);

        return id;
    }

    void ReBuild(int id){

        int ln = seg[id].sz;

        int pt1 = 0, pt2 = 0;
        Found_Ids(id, pt1, pt2);

        pt1 = 0, pt2 = 0;
        Set_Ids(0, ln - 1, pt1, pt2);
    }

    void Insert_DFS(int id, int k, int x){

        if (seg[id].sz == 1){

            Build(nc + 1, x);
            std::swap(seg[nc + 2], seg[id]);

            seg[id].lc = nc + 1;
            seg[id].rc = nc + 2;

            if (k == 1) std::swap(seg[id].lc, seg[id].rc);

            Merge(id);

            nc += 2;

            return;
        }

        Relax(id);

        int lc = seg[id].lc, rc = seg[id].rc;

        if (seg[lc].sz >= k) Insert_DFS(lc, k, x);
        else Insert_DFS(rc, k - seg[lc].sz, x);

        Merge(id);

        int sl = seg[lc].sz, sr = seg[rc].sz;
        if (std::max(sl, sr) == std::min(sl, sr) * 3) fr = id;

        // Slow And Normal Version:
        // if ((seg[id].sz & (seg[id].sz - 1)) == 0) fr = id;
    }

    void insert_index(int k, int x){

        if (nc == -1){

            Build(0, x);
            nc = 0;

            return;
        }

        fr = -1;
        Insert_DFS(0, k, x);

        if (fr != -1) ReBuild(fr);
    }

    void Update(int id, int l, int s, int e, ll x){

        int r = l + seg[id].sz - 1;

        if (s > r or e < l) return;
        if (s <= l and e >= r){

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

    void update_range(int l, int r, ll x){

        Update(0, 0, l, r, x);
    }

    ll Get(int id, int l, int s, int e){

        int r = l + seg[id].sz - 1;

        if (s > r or e < l) return 0;
        if (s <= l and e >= r) return seg[id].sum;

        Relax(id);

        int lc = seg[id].lc, rc = seg[id].rc;

        return Get(lc, l, s, e) + Get(rc, l + seg[lc].sz, s, e);
    }

    ll range_sum(int l, int r){

        return Get(0, 0, l, r);
    }
};

#endif
