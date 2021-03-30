#ifndef MEMORYMAPPEDFILEARRAY_HPP
#define MEMORYMAPPEDFILEARRAY_HPP
#include <stddef.h>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

template <typename T>
class MemoryMappedFileArray {
    public:
	int file;
	size_t block_size;
	T* mapped_array;
	size_t size = 0;
	size_t cap;
	MemoryMappedFileArray(const char* filename, size_t block_size = 4096);
	~MemoryMappedFileArray();
	void Realloc(size_t offset);
	void Resize(size_t size);
	void Add(T element);
	T& operator[] (size_t index);
};

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
    if(!this->mapped_array) {
	throw std::runtime_error("MemoryMappedFileArray::MemoryMappedFileArray(const char* filename, size_t block_size) Error: mapped_array is NULL");
    }
}

template <typename T>
MemoryMappedFileArray<T>::~MemoryMappedFileArray() {
    munmap(this->mapped_array, this->cap);
    ftruncate(this->file, this->size * sizeof(T));
    close(this->file);
}

template <typename T>
void MemoryMappedFileArray<T>::Realloc(size_t cap) {
    msync(this->mapped_array, this->cap, MS_SYNC);
    munmap(this->mapped_array, this->cap);
    this->cap = cap;
    ftruncate(this->file, this->cap);
    this->mapped_array = (T*)mmap(NULL, this->cap, PROT_READ | PROT_WRITE, MAP_SHARED, this->file, 0);
    if(!this->mapped_array) {
        throw std::runtime_error("MemoryMappedFileArray::Realloc(size_t cap) Error: mapped_array is NULL");
    }
}

template <typename T>
void MemoryMappedFileArray<T>::Add(T element) {
    if((size + 1) * sizeof(T) >= cap) {
	this->Realloc(this->cap + this->block_size); 
    }
    this->mapped_array[size] = element;
    this->size++;
}

template <typename T>
void MemoryMappedFileArray<T>::Resize(size_t size) {
    if(size > this->cap) {
	this->Realloc(((size * sizeof(T)) + 1) * this->block_size);
    }
    this->size = size;
}

template <typename T>
T& MemoryMappedFileArray<T>::operator[] (size_t index) {
    return mapped_array[index];
}

#endif // MEMORYMAPPEDFILEARRAY_HPP
