#include<iostream>
#include"AmortizedSegTree.hpp"

using namespace std;

int main(){

    // Fast IO
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int max_inserts = 2;
    SegTree tree(max_inserts);

    cout << "--- Testing Amortized Dynamic Segment Tree ---\n\n";

    // insert_index(index, value)
    // Its 0-Indexed

    tree.insert_index(0, 10); // [10]
    tree.insert_index(1, 20); // [10, 20]


    // need more space
    tree.Add_More_Space(100000); // Space Change To 100000 From 2


    tree.insert_index(0, 5);  // [5, 10, 20]
    tree.insert_index(3, 30); // [5, 10, 20, 30]

    cout << "Length of array after insertions: " << tree.Get_Length() << "\n";


    // range_sum(L, R) - 0-Indexed
    cout << "Sum of elements from 0 to 2: " << tree.range_sum(0, 2) << "\n"; // (5 + 10 + 20) = 35


    // update_range(L, R, value) - 0-Indexed
    cout << "\nAdding +100 to range [1, 3]...\n";
    tree.update_range(1, 3, 100);


    cout << "Value of element at index 1: " << tree.range_sum(1, 1) << "\n"; // 110
    cout << "Total sum of the array: " << tree.range_sum(0, 3) << "\n"; // 365

    return 0;
}
