#if defined (unix)
typedef int fd_t;
#endif

#if defined (WIN32)
typedef struct fd {
  void* hFile;
  void* hMapping;
} fd_t;
#endif


template <typename T>
class MemoryMappedFileArray {
    public:
	fd_t file;
	size_t block_size;
	T* mapped_array;
	size_t size;
	size_t cap;
	MemoryMappedFileArray(const char* filename);
	~MemoryMappedFileArray();
	bool realloc(size_t size);
	bool add(T element);
	T& operator[] (size_t index);
}


// UNIX
#if defined (unix)
#include <unistd.h>
MemoryMappedFileArray::MemoryMappedFileArray(const char* filename, size_t block_size = 4096) {
    this->file = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    this->block_size = block_size;
    struct stat st;
    fstat(this->file, &st); 
    this->size = st.st_size / sizeof(T);
    this->cap = ((this->size * sizeof(T) / this->block_size) + 1) * this->block_size;
    ftruncate(this->file, this->cap);
    this->mapped_array = mmap(nullptr, this->cap, PROT_READ | PROT_WRITE, MAP_PRIVATE, this->file, 0);
}

MemoryMappedFileArray::~MemoryMappedFileArray() {
    munmap(this->mapped_array, this->cap);
    ftruncate(this->file, this->size * sizeof(T));
    close(this->file);
}

MemoryMappedFileArray::realloc(size_t cap) {
    munmap(this->mapped_array, this->cap);
    this->cap = cap;
    ftruncate(this->file, this->cap);
    this->mapped_array = mmap(nullptr, this->cap, PROT_READ | PROT_WRITE, MAP_PRIVATE, this->file, 0);
    if(mapped_array) {
	return true;
    }
    else {
	return false;
    }
}

bool add(T element) {
    if((size + 1) * sizeof(T) == cap) {
	if(this->resize(this->cap + this->block_size)) {
	    this->mapped_array[size] = element;
	    return true;
	}
	else {
	    return false;
	}
    }
    this->mapped_array[size] = element;
    return true;
}

T& MemoryMappedFileArray::operator[] (size_t index) {
    return mapped_array[index];
}
#endif

//WINDOWS
