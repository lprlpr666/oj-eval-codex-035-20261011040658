#ifndef MYSTRING_HPP
#define MYSTRING_HPP

#include <cstddef>
#include <cstring>
#include <stdexcept>

/**
 * Simple string implementation with Small String Optimization (SSO).
 * Supports basic operations required by the problem statement.
 */
class MyString {
private:
    // Union for SSO buffer or heap pointer.
    union Storage {
        char* heap_ptr;          // dynamically allocated buffer (null terminated)
        char small_buffer[16];   // SSO buffer, holds up to 15 chars + '\0'
        Storage() { heap_ptr = nullptr; }
    } storage;

    std::size_t len;   // number of characters (excluding null terminator)
    std::size_t cap;   // capacity (excluding null terminator). For SSO cap is 15.
    bool is_small;     // true if using SSO buffer

    // Helper to get pointer to data (const)
    const char* data_ptr() const {
        return is_small ? storage.small_buffer : storage.heap_ptr;
    }
    // Helper to get pointer to data (non-const)
    char* data_ptr() {
        return is_small ? storage.small_buffer : storage.heap_ptr;
    }

    // Allocate heap storage with given capacity (must be > 15)
    void allocate_heap(std::size_t new_cap) {
        char* new_buf = new char[new_cap + 1]; // +1 for null terminator
        new_buf[0] = '\0';
        storage.heap_ptr = new_buf;
        cap = new_cap;
        is_small = false;
    }

    // Switch from heap to small buffer when size fits.
    void switch_to_small() {
        if (!is_small) {
            // copy data to small buffer
            std::memcpy(storage.small_buffer, storage.heap_ptr, len);
            storage.small_buffer[len] = '\0';
            delete[] storage.heap_ptr;
            is_small = true;
            cap = 15;
        }
    }

    // Ensure we have enough capacity for additional n characters.
    void ensure_capacity(std::size_t additional) {
        std::size_t needed = len + additional;
        if (needed <= cap) return; // already enough
        // Calculate new capacity (grow factor 1.5)
        std::size_t new_cap = cap ? static_cast<std::size_t>(cap * 1.5) : 1;
        while (new_cap < needed) new_cap = new_cap * 2;
        // allocate new buffer
        char* new_buf = new char[new_cap + 1];
        // copy existing data
        std::memcpy(new_buf, data_ptr(), len);
        new_buf[len] = '\0';
        // free old heap if needed
        if (!is_small) delete[] storage.heap_ptr;
        storage.heap_ptr = new_buf;
        cap = new_cap;
        is_small = false;
    }

public:
    // Default constructor – empty string.
    MyString() : len(0), cap(15), is_small(true) {
        storage.small_buffer[0] = '\0';
    }

    // Constructor from C‑string.
    explicit MyString(const char* s) {
        if (!s) {
            // treat null as empty string
            len = 0; cap = 15; is_small = true; storage.small_buffer[0] = '\0';
            return;
        }
        std::size_t l = std::strlen(s);
        len = l;
        if (l <= 15) {
            is_small = true;
            cap = 15;
            std::memcpy(storage.small_buffer, s, l + 1);
        } else {
            is_small = false;
            // allocate a bit larger than needed (allow growth factor later)
            cap = l;
            allocate_heap(cap);
            std::memcpy(storage.heap_ptr, s, l + 1);
        }
    }

    // Copy constructor.
    MyString(const MyString& other) : len(other.len), cap(other.cap), is_small(other.is_small) {
        if (other.is_small) {
            std::memcpy(storage.small_buffer, other.storage.small_buffer, len + 1);
        } else {
            storage.heap_ptr = new char[cap + 1];
            std::memcpy(storage.heap_ptr, other.storage.heap_ptr, len + 1);
        }
    }

    // Move constructor.
    MyString(MyString&& other) noexcept : len(other.len), cap(other.cap), is_small(other.is_small) {
        if (other.is_small) {
            std::memcpy(storage.small_buffer, other.storage.small_buffer, len + 1);
        } else {
            storage.heap_ptr = other.storage.heap_ptr;
            other.storage.heap_ptr = nullptr;
        }
        // leave other in valid empty state
        other.len = 0;
        other.cap = 15;
        other.is_small = true;
        other.storage.small_buffer[0] = '\0';
    }

    // Move assignment.
    MyString& operator=(MyString&& other) noexcept {
        if (this != &other) {
            // clean current
            if (!is_small) delete[] storage.heap_ptr;
            len = other.len;
            cap = other.cap;
            is_small = other.is_small;
            if (other.is_small) {
                std::memcpy(storage.small_buffer, other.storage.small_buffer, len + 1);
            } else {
                storage.heap_ptr = other.storage.heap_ptr;
                other.storage.heap_ptr = nullptr;
            }
            // reset other
            other.len = 0;
            other.cap = 15;
            other.is_small = true;
            other.storage.small_buffer[0] = '\0';
        }
        return *this;
    }

    // Copy assignment.
    MyString& operator=(const MyString& other) {
        if (this != &other) {
            // reuse existing storage if possible
            if (other.is_small) {
                if (!is_small) delete[] storage.heap_ptr;
                is_small = true;
                cap = 15;
                std::memcpy(storage.small_buffer, other.storage.small_buffer, other.len + 1);
            } else {
                // need heap storage
                if (is_small || cap < other.len) {
                    if (!is_small) delete[] storage.heap_ptr;
                    allocate_heap(other.cap);
                }
                std::memcpy(storage.heap_ptr, other.storage.heap_ptr, other.len + 1);
                is_small = false;
                cap = other.cap;
            }
            len = other.len;
        }
        return *this;
    }

    // Destructor.
    ~MyString() {
        if (!is_small) delete[] storage.heap_ptr;
    }

    const char* c_str() const { return data_ptr(); }

    std::size_t size() const { return len; }

    std::size_t capacity() const { return cap; }

    void reserve(std::size_t new_capacity) {
        if (new_capacity <= cap) return;
        if (new_capacity <= 15) {
            // still fits in SSO, just set cap to 15
            cap = 15;
            return;
        }
        // allocate new heap buffer
        char* new_buf = new char[new_capacity + 1];
        std::memcpy(new_buf, data_ptr(), len);
        new_buf[len] = '\0';
        if (!is_small) delete[] storage.heap_ptr;
        storage.heap_ptr = new_buf;
        cap = new_capacity;
        is_small = false;
    }

    void resize(std::size_t new_size) {
        if (new_size == len) return;
        if (new_size < len) {
            // shrink
            len = new_size;
            data_ptr()[len] = '\0';
            // optionally shrink capacity if far smaller
            if (!is_small && new_size <= 15) {
                // move to small buffer
                switch_to_small();
            }
            return;
        }
        // need to grow
        ensure_capacity(new_size - len);
        // fill new characters with '\0'
        std::memset(data_ptr() + len, '\0', new_size - len);
        len = new_size;
        data_ptr()[len] = '\0';
    }

    char& operator[](std::size_t index) {
        // No bounds check as per std::string behavior
        return data_ptr()[index];
    }

    const char& operator[](std::size_t index) const {
        return data_ptr()[index];
    }

    const char& at(std::size_t pos) const {
        if (pos >= len) throw std::out_of_range("MyString::at out of range");
        return data_ptr()[pos];
    }

    MyString operator+(const MyString& rhs) const {
        MyString result;
        result.len = len + rhs.len;
        if (result.len <= 15) {
            result.is_small = true;
            result.cap = 15;
            std::memcpy(result.storage.small_buffer, data_ptr(), len);
            std::memcpy(result.storage.small_buffer + len, rhs.data_ptr(), rhs.len + 1);
        } else {
            result.is_small = false;
            result.cap = result.len; // allocate exactly needed
            result.allocate_heap(result.cap);
            std::memcpy(result.storage.heap_ptr, data_ptr(), len);
            std::memcpy(result.storage.heap_ptr + len, rhs.data_ptr(), rhs.len + 1);
        }
        return result;
    }

    void append(const char* str) {
        if (!str) return;
        std::size_t add_len = std::strlen(str);
        ensure_capacity(add_len);
        std::memcpy(data_ptr() + len, str, add_len + 1);
        len += add_len;
    }

    // Forward declaration for iterator cross-comparison
    class const_iterator;
    // Simple iterator based on raw pointer.
    class iterator {
        char* ptr;
    public:
        explicit iterator(char* p = nullptr) : ptr(p) {}
        char* get() const { return ptr; }
        iterator& operator++() { ++ptr; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++ptr; return tmp; }
        iterator& operator--() { --ptr; return *this; }
        iterator operator--(int) { iterator tmp = *this; --ptr; return tmp; }
        char& operator*() const { return *ptr; }
        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
        // cross-comparison with const_iterator
        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
    };

    // Simple const iterator based on raw pointer.
    class const_iterator {
        const char* ptr;
    public:
        explicit const_iterator(const char* p = nullptr) : ptr(p) {}
        const char* get() const { return ptr; }
        const_iterator& operator++() { ++ptr; return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; ++ptr; return tmp; }
        const_iterator& operator--() { --ptr; return *this; }
        const_iterator operator--(int) { const_iterator tmp = *this; --ptr; return tmp; }
        const char& operator*() const { return *ptr; }
        bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
        // allow comparison with iterator
        bool operator==(const iterator& other) const { return ptr == other.get(); }
        bool operator!=(const iterator& other) const { return ptr != other.get(); }
    };


    iterator begin() { return iterator(data_ptr()); }
    iterator end() { return iterator(data_ptr() + len); }
    const_iterator cbegin() const { return const_iterator(data_ptr()); }
    const_iterator cend() const { return const_iterator(data_ptr() + len); }
};

// Define cross-iterator comparisons after class definition.
inline bool MyString::iterator::operator==(const MyString::const_iterator& other) const { return ptr == other.get(); }
inline bool MyString::iterator::operator!=(const MyString::const_iterator& other) const { return ptr != other.get(); }

#endif // MYSTRING_HPP
