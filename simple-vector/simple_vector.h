#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) : capacity(capacity_to_reserve) {};
    size_t capacity;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return {capacity_to_reserve};
}

template<typename Type>
class SimpleVector {
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;


    SimpleVector(const SimpleVector &other) : size_(other.size_), capacity_(other.size_ * 2) {
        ArrayPtr<Type> new_item(capacity_);
        std::copy(other.begin(), other.end(), new_item.Get());
        items_.swap(new_item);
    }

    SimpleVector(SimpleVector &&other) noexcept: size_(other.size_),
                                                 capacity_(other.capacity_) {
        items_ = std::move(other.items_);
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector(size_t size, const Type &value) : items_(size), size_(size), capacity_(size_) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) :items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }

    explicit SimpleVector(size_t size) : items_(size), size_(size), capacity_(size_) {
        std::fill(begin(), end(), Type());
    }

    explicit SimpleVector(ReserveProxyObj new_capacity) : items_(new_capacity.capacity),
                                                          capacity_(new_capacity.capacity) {}

    SimpleVector &operator=(const SimpleVector &rhs) {
        if (*this != rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector operator=(SimpleVector &&rhs) {
        if (*this != rhs || !rhs.IsEmpty()) {
            SimpleVector tmp = std::move(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type &item) {
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ * 2;
            if (new_capacity == 0) {
                ++new_capacity;
            }
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        if (size_ == 0) {
            size_t new_capacity;
            capacity_ == 0 ? new_capacity = 1 : new_capacity = capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type &&item) {
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ * 2;
            if (new_capacity == 0) {
                ++new_capacity;
            }
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        if (size_ == 0) {
            size_t new_capacity;
            capacity_ == 0 ? new_capacity = 1 : new_capacity = capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type &value) {
        assert(pos >= begin() && pos <= end());
        if (pos == begin() + size_) {
            PushBack(value);
            return begin() + size_ - 1;
        }
        if (capacity_ == size_) {
            capacity_ *= 2;
            ArrayPtr<Type> new_items(capacity_);
            std::copy(begin(), pos, new_items.Get());
            std::copy(pos, end(), new_items.Get() + (pos - begin()) + 1);
            new_items[(pos - begin())] = value;
            items_.swap(new_items);
        } else {
            std::copy_backward(pos, end(), end() + 1);
            items_[pos - begin()] = value;
        }
        ++size_;
        return begin() + (pos - begin());
    }

    Iterator Insert(ConstIterator pos, Type &&value) {
        assert(pos >= begin() && pos <= end());
        if (pos == begin() + size_) {
            PushBack(std::move(value));
            return begin() + size_ - 1;
        }
        if (capacity_ == size_) {
            capacity_ *= 2;
            ArrayPtr<Type> new_items(capacity_);
            std::move(SimpleVector<Type>::Iterator(begin()), SimpleVector<Type>::Iterator(pos), SimpleVector<Type>::Iterator(new_items.Get()));
            std::move(SimpleVector<Type>::Iterator(pos), end(), SimpleVector<Type>::Iterator(new_items.Get() + (pos - begin()) + 1));
            new_items[(pos - begin())] = std::move(value);
            items_.swap(new_items);
        } else {
            std::move_backward(SimpleVector<Type>::Iterator(pos), SimpleVector<Type>::Iterator(end()), SimpleVector<Type>::Iterator(end() + 1));
            items_[pos - begin()] = std::move(value);
        }
        ++size_;
        return begin() + (pos - begin());
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ != 0);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);
        std::move(begin() + dist + 1, end(), begin() + dist);
        --size_;
        return items_.Get() + dist;
    }


    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ < new_capacity) {
            ArrayPtr<Type> new_items(new_capacity);
            std::copy(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    // Возвращает количество элементов в массиве
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    [[nodiscard]] size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
            return;
        }
        if (new_size > capacity_) {
            ArrayPtr<Type> new_items(new_size);
            std::move(begin(), end(), new_items.Get());
            for (size_t i = size_; i < new_size; ++i) {
                new_items[i] = Type();
            }
            items_ = std::move(new_items);
            capacity_ = new_size * 2;
            size_ = new_size;
        } else if (new_size <= capacity_) {
            ArrayPtr<Type> new_items(new_size);
            std::move(begin(), end(), new_items.Get());
            if (new_size > size_) {
                for (size_t i = size_; i < new_size; ++i) {
                    new_items[i] = Type();
                }
            }
            items_ = std::move(new_items);
            size_ = new_size;
        }
    }

    Iterator begin() noexcept {
        return Iterator{items_.Get()};
    }

    Iterator end() noexcept {
        return Iterator{items_.Get() + size_};
    }

    ConstIterator begin() const noexcept {
        return ConstIterator{items_.Get()};

    }

    ConstIterator end() const noexcept {
        return ConstIterator{items_.Get() + size_};
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator{items_.Get()};
    }

    ConstIterator cend() const noexcept {
        return ConstIterator{items_.Get() + size_};
    }


private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};


template<typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    if (lhs.GetSize() != rhs.GetSize()) return false;
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(lhs == rhs);
}

template<typename Type>
bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(rhs > lhs);
}


template<typename Type>
bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return rhs < lhs;
}

template<typename Type>
bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(rhs < lhs);;
}

