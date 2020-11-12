/**
 * This wraps the StringArray.hpp class for SWIG usage,
 * adding the basic C-style wrappers needed to make it 
 * usable for the users of the low-level lightgbmJNI API.
 */

%{
#include "../swig/ChunkedArray.hpp"
%}

//%ignore ChunkedArray;


//// Not uset ad the moment!!!!!!
%inline %{

    typedef void* ChunkedArrayHandle;

    /**
     * @brief Creates a new StringArray and returns its handle.
     * 
     * @param num_strings number of strings to store.
     * @param string_size the maximum number of characters that can be stored in each string.
     * @return ChunkedArrayHandle or nullptr in case of allocation failure.
     */
    template<typename T>
    ChunkedArrayHandle ChunkedArrayHandle_create(size_t chunk_size) {
        try {
            return new ChunkedArray<T>(chunk_size);
        } catch (std::bad_alloc &e) {
            return nullptr;
        }
    }


    /**
     * @brief Free the StringArray object.
     * 
     * @param handle StringArray handle.
     */
    template<typename T>
    void ChunkedArrayHandle_free(ChunkedArrayHandle handle)
    {
        delete reinterpret_cast<ChunkedArray<T> *>(handle);
    }



    /**
     * For the end user to extract a specific string from the StringArray object.
     * 
     * @param handle StringArray handle.
     * @param index index of the string to retrieve from the array.
     * @return raw pointer to string at index, or nullptr if out of bounds.
     */
    template<typename T>
    T ChunkedArrayHandle_getitem(ChunkedArrayHandle handle, size_t chunk, size_t index, T on_fail_value)
    {
        return reinterpret_cast<ChunkedArray<T> *>(handle)->getitem(chunk, index, on_fail_value);
    }



    /**
     * @brief Replaces one string of the array at index with the new content.
     * 
     * @param handle StringArray handle.
     * @param index Index of the string to replace
     * @param new_content The content to replace
     * @return 0 (success) or -1 (error) in case of out of bounds index or too large content.
     */
    template <typename T>
    int ChunkedArrayHandle_setitem(ChunkedArrayHandle handle, size_t chunk, size_t index, T value)
    {
        return reinterpret_cast<ChunkedArray<T> *>(handle)->setitem(chunk, index, value);
    }


    template<typename T>
    void ChunkedArrayHandle_add(ChunkedArrayHandle handle, T value)
    {
        reinterpret_cast<ChunkedArray<T> *>(handle)->add(value);
    }    

%}

%include "../swig/ChunkedArray.hpp"

%template(int_CA) ChunkedArray<int>;
%template(float_CA) ChunkedArray<float>;
%template(double_CA) ChunkedArray<double>;

/*

    %template(floatChunkedArrayHandle_free) ChunkedArrayHandle_free<float>;
    %template(doubleChunkedArrayHandle_free) ChunkedArrayHandle_free<double>;
    %template(intChunkedArrayHandle_free) ChunkedArrayHandle_free<int>;

    %template(floatChunkedArrayHandle_getitem) ChunkedArrayHandle_getitem<float>;
    %template(doubleChunkedArrayHandle_getitem) ChunkedArrayHandle_getitem<double>;
    %template(intChunkedArrayHandle_getitem) ChunkedArrayHandle_getitem<int>;

    %template(floatChunkedArrayHandle_create) ChunkedArrayHandle_create<float>;
    %template(doubleChunkedArrayHandle_create) ChunkedArrayHandle_create<double>;
    %template(intChunkedArrayHandle_create) ChunkedArrayHandle_create<int>;

    %template(floatChunkedArrayHandle_setitem) ChunkedArrayHandle_setitem<float>;
    %template(doubleChunkedArrayHandle_setitem) ChunkedArrayHandle_setitem<double>;
    %template(intChunkedArrayHandle_setitem) ChunkedArrayHandle_setitem<int>;

    %template(floatChunkedArrayHandle_add) ChunkedArrayHandle_add<float>;
    %template(doubleChunkedArrayHandle_add) ChunkedArrayHandle_add<double>;
    %template(intChunkedArrayHandle_add) ChunkedArrayHandle_add<int>;

    */