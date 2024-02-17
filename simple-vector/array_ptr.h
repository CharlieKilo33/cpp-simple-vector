// вставьте сюда ваш код для класса ArrayPtr
// внесите в него изменения, 
// которые позволят реализовать move-семантику

#include <cstddef>
#include <utility>

template<typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(Type *raw_ptr) noexcept: raw_ptr_(raw_ptr) {}

    explicit ArrayPtr(size_t size) {
        (size == 0) ? raw_ptr_ = nullptr : raw_ptr_ = new Type[size];
    }

    ArrayPtr(ArrayPtr &&other) noexcept: raw_ptr_(other.raw_ptr_) {
        other.raw_ptr_ = nullptr;
    }

    ArrayPtr &operator=(ArrayPtr &&other) noexcept {
        if (this != &other) {
            delete[] raw_ptr_;
            raw_ptr_ = other.raw_ptr_;
            other.raw_ptr_ = nullptr;
        }
        return *this;
    }

    ArrayPtr(const ArrayPtr &) = delete;

    ArrayPtr &operator=(const ArrayPtr &) = delete;

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other){
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type *raw_ptr_ = nullptr;
};