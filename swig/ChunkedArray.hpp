
#ifndef __CHUNKED_ARRAY_H__
#define __CHUNKED_ARRAY_H__

#include <new>
#include <vector>
#include <algorithm>

#include <stdint.h>
#include <assert.h>

/**
 * Container that manages a dynamic array of fixed-length chunks.
 *
 * The class also takes care of allocation & release of the underlying
 * memory.
 * 
 * Note: When using this for `LGBM_DatasetCreateFromMats` use a 
 *       chunk_size multiple of #num_cols for your dataset, so each chunk
 *       contains "complete" instances.
 * 
 * The easiest way to use is:
 *  0. ChunkedArray(chunk_size) # Choose appropriate size
 *  1. add(value) # as many times as you want (will generate chunks as needed)
 *  2. data()     # retrieves a T** pointer (useful for `LGBM_DatasetCreateFromMats`).
 * 
 * Can then find useful the following query methods: 
 *  - get_added_count() # total count of added elements 
 *  - get_chunks_count() # how many chunks are currently allocated.
 *  - get_current_chunk_added_count() # for the last add() chunk, how many items are there
 *  - get_chunk_size() # constant of initialization. retrieves chunk_size from constructor call.
 * 
 * 
 * With those you can generate int32_t sizes[]. (last chunk can be smaller than chunk_size)
 * 
 * =======================================
 * 
 * Note: For advanced users (useful for reading in parallel)
 *       You can also manually call new_chunk() as many times as you want, and setitem(chunk, idx, value) explicitly.
 * 
 */
template <class T>
class ChunkedArray
{
  public:
    ChunkedArray(size_t chunk_size)
      : _chunk_size(chunk_size), _current_chunks_idx(0), _current_chunk_idx(0)
    {
       new_chunk();
    }

    ~ChunkedArray()
    {
        release();
    }

    /**
     * Adds a value to the chunks sequentially. 
     * If the last chunk is full it creates a new one and appends to it.     
     */
    void add(T value) {
        if (! within_bounds(_current_chunks_idx, _current_chunk_idx)) {
            new_chunk();
            _current_chunks_idx += 1;
            _current_chunk_idx = 0;            
        } 

        assert (setitem(_current_chunks_idx, _current_chunk_idx, value) == 0);        
        _current_chunk_idx += 1;
    }

    size_t get_added_count() {
        return _current_chunks_idx * _chunk_size + _current_chunk_idx;
    }

    size_t get_chunks_count() {
        return _chunks.size();
    }

    size_t get_current_chunk_added_count() {
        return _current_chunk_idx;
    }

    size_t get_chunk_size() {
        return _chunk_size;
    }

    /**
     * Returns the pointer to the raw array.
     * Notice its size is greater than the number of stored strings by 1.
     *
     * @return T** pointer to raw data (null-terminated).
     */
    T **data() noexcept
    {
        return _chunks.data();
    }

    /**
     * Return value from array of chunks.
     *
     * @param chunks_index index of the chunk
     * @param index index within chunk
     * @param on_fail_value sentinel value. If out of bounds returns that value.
     * 
     * @return pointer or nullptr if index is out of bounds.
     */
    T getitem(size_t chunks_index, size_t index, T on_fail_value) noexcept
    {
        if (within_bounds(chunks_index, index))
            return _chunks[chunks_index][index];
        else
            return on_fail_value;
    }

    /**
     *
     * @param chunks_index index of the chunk
     * @param index index within chunk
     * @param value value to store
     *
     * @return 0 = success, -1 = out of bounds access.
     */
    int setitem(size_t chunks_index, size_t index, T value) noexcept
    {
        if (within_bounds(chunks_index, index))
        {
            _chunks[chunks_index][index] = value;
            return 0;
        } else {
            return -1;
        }
    }

    /**
     * Deletes the allocated chunks.
     */
    void release() noexcept
    {
        std::for_each(_chunks.begin(), _chunks.end(), [](T* c) { delete[] c; });
        _sizes.clear();
        _sizes.shrink_to_fit();
        _chunks.clear();
        _chunks.shrink_to_fit();
        _current_chunks_idx = 0;
        _current_chunk_idx = 0;
    }

    inline bool within_bounds(size_t chunks_index, size_t index) {
        return (chunks_index < _chunks.size()) && (index < _chunk_size);
    }

    /**
     * Allocate an array of fixed-length strings.
     *
     * Since a NULL-terminated array is required by SWIG's `various.i`,
     * the size of the array is actually `num_elements + 1` but only
     * num_elements are filled.
     *
     * @param num_elements Number of strings to store in the array.
     * @param string_size The size of each string in the array.
     */
    void new_chunk()
    {       
        _chunks.push_back(new (std::nothrow) T[_chunk_size]);

        // Check memory allocation success:
        if (! _chunks[_chunks.size()-1]) {
            release();
            throw std::bad_alloc();
        }
    }

  private:

    size_t _current_chunks_idx; //<! Index of chunks
    size_t _current_chunk_idx;  //<! Index within chunk

    std::vector<int32_t> _sizes;

    const size_t _chunk_size;
    std::vector<T*> _chunks;
};

#endif // __CHUNKED_ARRAY_H__
