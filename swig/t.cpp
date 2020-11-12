#include <iostream>
#include "ChunkedArray.hpp"
#include <stdio.h>
using namespace std;

using intChunkedArray=ChunkedArray<int>;
using doubleChunkedArray = ChunkedArray<double>;

int out_of_bounds = 2; // test get outside bounds.

template<class T>
void print_container_stats(ChunkedArray<T> &ca) {
    printf("\n\nContainer stats: %ld chunks of size %ld with %ld item(s) on last chunk.\n\n",
        ca.get_chunks_count(),
        ca.get_chunk_size(),
        ca.get_current_chunk_added_count()
    );
}

int main() {
    const size_t chunk_size = 3;
    intChunkedArray ca = ChunkedArray<int>(chunk_size);

    for (int i = 1; i <= 7; ++i) {
        ca.add(i);
    }
    print_container_stats(ca);

    int chunk = 0; 
    int pos = 0;
    for (int i = 0; i <= ca.get_added_count() + out_of_bounds; ++i) {        

        cout << "@(" << chunk << "," << pos << ") = " << ca.getitem(chunk, pos, 10) << endl;        
        ++pos;

        if (pos == ca.get_chunk_size()) {
            ++chunk;
            pos = 0; 
        }
    }
    
    ca.release(); ca.release(); print_container_stats(ca); // Test double free behaviour.
    cout << "Done!" << endl;
}