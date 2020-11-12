#include <iostream>
#include "ChunkedArray.hpp"
#include <stdio.h>
using namespace std;

using intChunkedArray=ChunkedArray<int>;
using doubleChunkedArray = ChunkedArray<double>;

int out_of_bounds = 4; // test get outside bounds.

template<class T>
size_t get_merged_array_size(ChunkedArray<T> &ca) {
    if (ca.empty()) {
        return 0;
    } else {
        size_t prior_chunks_total_size = (ca.get_chunks_count() - 1) * ca.get_chunk_size();
        return prior_chunks_total_size + ca.get_current_chunk_added_count();
    }
}

template<class T>
void print_container_stats(ChunkedArray<T> &ca) {
    printf("\n\nContainer stats: %ld chunks of size %ld with %ld item(s) on last chunk (#elements=%ld).\n"
           " > Should result in single array of size %ld.\n\n",
        ca.get_chunks_count(),
        ca.get_chunk_size(),
        ca.get_current_chunk_added_count(),
        ca.get_added_count(),
        get_merged_array_size(ca)
    );
}

template <typename T>
void print_data(ChunkedArray<T> &x) {
  size_t chunk_size = x.get_chunk_size();
  T **data = x.data();

  int chunk = 0;
  int pos = 0;
  cout << "Printing from T** data(): \n";
  for (int i = 0; i < x.get_added_count(); ++i) {
      cout << data[chunk][pos] << " ";

      ++pos;
      if (pos == chunk_size) {
          pos = 0;
          ++chunk;
          cout << "\n";
      }
  }
  cout << "\n^ Print complete ^\n";
}

template <typename T>
void print_void_data(ChunkedArray<T> &x) {
  size_t chunk_size = x.get_chunk_size();
  T **data = reinterpret_cast<T**>(x.void_data());

  int chunk = 0;
  int pos = 0;
  cout << "Printing from reinterpret_cast<T**>(void_data()):\n";
  for (int i = 0; i < x.get_added_count(); ++i) {
      cout << data[chunk][pos] << " ";

      ++pos;
      if (pos == chunk_size) {
          pos = 0;
          ++chunk;
          cout << "\n";
      }
  }
  cout << "\n^ Print complete ^\n";
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
    for (int i = 0; i < ca.get_added_count() + out_of_bounds; ++i) {        

        bool within_added = i < ca.get_added_count();
        bool within_bounds = ca.within_bounds(chunk, pos);
        cout << "@(" << chunk << "," << pos << ") = " << ca.getitem(chunk, pos, 10) 
        << " " << within_added << " " << within_bounds << endl;      

        ++pos;

        if (pos == ca.get_chunk_size()) {
            ++chunk;
            pos = 0; 
        }
    }
    
    print_data<int>(ca);
    print_void_data<int>(ca);

    ca.release(); ca.release(); print_container_stats(ca); // Test double free behaviour.
    cout << "Done!" << endl;
}