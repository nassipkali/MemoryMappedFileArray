#ifndef MEMORYMAPPEDFILEARRAY_HPP
#define MEMORYMAPPEDFILEARRAY_HPP
#include <stddef.h>
#if defined (unix)
typedef int fd_t;
#endif

#if defined (WIN32)
typedef struct fd {
  void* file_handle;
  void* mapping_handle;
} fd_t;
#endif


template <typename T>
class MemoryMappedFileArray {
    public:
	fd_t file;
	size_t block_size;
	T* mapped_array;
	size_t size = 0;
	size_t cap;
	MemoryMappedFileArray(const char* filename, size_t block_size = 4096);
	~MemoryMappedFileArray();
	bool Realloc(size_t offset);
	bool Resize(size_t size);
	bool Add(T element);
	T& operator[] (size_t index);
};


// UNIX
#if defined (unix)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

template <typename T>
MemoryMappedFileArray<T>::MemoryMappedFileArray(const char* filename, size_t block_size) {
    this->file = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    this->block_size = block_size;
    struct stat st;
    fstat(this->file, &st); 
    this->size = st.st_size / sizeof(T);
    this->cap = ((this->size * sizeof(T) / this->block_size) + 1) * this->block_size;
    ftruncate(this->file, this->cap);
    this->mapped_array = (T*)mmap(NULL, this->cap, PROT_READ | PROT_WRITE, MAP_SHARED, this->file, 0);
}

template <typename T>
MemoryMappedFileArray<T>::~MemoryMappedFileArray() {
    munmap(this->mapped_array, this->cap);
    ftruncate(this->file, this->size * sizeof(T));
    close(this->file);
}

template <typename T>
bool MemoryMappedFileArray<T>::Realloc(size_t cap) {
    msync(this->mapped_array, this->cap, MS_SYNC);
    munmap(this->mapped_array, this->cap);
    this->cap = cap;
    ftruncate(this->file, this->cap);
    this->mapped_array = (T*)mmap(NULL, this->cap, PROT_READ | PROT_WRITE, MAP_SHARED, this->file, 0);
    if(mapped_array) {
        return true;
    }
    else {
        return false;
    }
}
#endif

//WINDOWS
#if defined (WIN32)
template <typename T>
MemoryMappedFileArray<T>::MemoryMappedFileArray(const char* filename, size_t block_size = 4096) {
    this->file.file_handle = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD file_size = GetFileSize(this->file.file_handle, nullptr);
    this->size = file_size;
    this->cap = (this->size + sizeof(T) / block_size + 1) * block_size;
    FileResize(this->cap);
    this->file.mapping_handle = CreateFileMapping(this->file.file_handle, nullptr, PAGE_READWRITE, 0, (DWORD)this->cap, nullptr);
    this->mapped_array = (T*)MapViewOfFile(this->file.mapping_handle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, (DWORD)this->cap);
}

template <typename T>
MemoryMappedFileArray<T>::~MemoryMappedFileArray() {
    UnMapViewOfFile(this->mapped_array);
    SetFilePointer(this->file.file_handle, this->size, nullptr, FILE_BEGIN);
    SetEndOfFile(this->file.file_handle);
}

template <typename T>
bool MemoryMappedFileArray<T>::Realloc(size_t cap) {
    this->cap = cap;
    UnMapViewOfFile(this->mapped_array);
    CloseHandle(this->file.mapping_handle);
    SetFilePointer(this->file.file_handle, this->cap, nullptr, FILE_BEGIN);
    SetEndOfFile(this->file.file_handle);
    this->file.mapping_handle = CreateFileMapping(this->file.file_handle, nullptr, PAGE_READWRITE, 0, (DWORD)this->cap, nullptr);
    this->mapped_array = (T*)MapViewOfFile(this->file.mapping_handle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, (DWORD)this->cap);
}
#endif

template <typename T>
bool MemoryMappedFileArray<T>::Add(T element) {
    if((size + 1) * sizeof(T) >= cap) {
	if(this->Realloc(this->cap + this->block_size)) {
	    this->mapped_array[size] = element;
	    this->size++;
	    return true;
	}
	else {
	    return false;
	}
    }
    this->mapped_array[size] = element;
    this->size++;
    return true;
}

template <typename T>
bool MemoryMappedFileArray<T>::Resize(size_t size) {
    if(size > this->cap) {
	if(!this->Realloc(((size * sizeof(T)) + 1) * this->block_size)) {
	    return false;
	}
    }
    this->size = size;
    return true;
}

template <typename T>
T& MemoryMappedFileArray<T>::operator[] (size_t index) {
    return mapped_array[index];
}

#endif // MEMORYMAPPEDFILEARRAY_HPP
