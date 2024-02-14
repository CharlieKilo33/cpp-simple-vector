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


    SimpleVector(const SimpleVector &other) {
        size_ = std::move(other.size_);
        capacity_ = other.size_ * 2;
        items_ = new Type[capacity_];
        std::copy(other.items_, other.items_ + other.size_, items_);
    }

    SimpleVector(SimpleVector &&other) noexcept {
        size_ = other.size_;
        capacity_ = other.size_ * 2;
        items_ = new Type[capacity_];
        std::move(other.items_, other.items_ + other.size_, items_);
        other.size_ = 0;
        other.capacity_ = 0;
    }

    explicit SimpleVector(ReserveProxyObj new_capacity) {
        if (capacity_ < new_capacity.capacity) {
            auto new_item = new Type[size_];
            std::copy(SimpleVector<Type>::Iterator(items_), SimpleVector<Type>::Iterator(items_ + size_), new_item);
            delete[] items_;
            capacity_ = new_capacity.capacity;
        } else {
            capacity_ = new_capacity.capacity;
        }
    }

    SimpleVector &operator=(const SimpleVector &rhs) {
        this->size_ = rhs.size_;
        this->capacity_ = rhs.capacity_ * 2;
        items_ = new Type[size_];
        std::copy(items_, items_ + size_, rhs.items_);
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type &item) {
        if (size_ == capacity_) {
            Expand(capacity_ * 2);
        }
        if (size_ == 0) {
            items_ = new Type[capacity_];
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type &&item) {
        if (size_ == capacity_) {
            size_t new_capacity;
            capacity_ == 0 ? new_capacity = 1 : new_capacity = capacity_ * 2;
            auto new_items = new Type[new_capacity];
            std::move(SimpleVector<Type>::Iterator(items_), SimpleVector<Type>::Iterator(items_ + size_),
                      SimpleVector<Type>::Iterator(new_items));
            delete[] items_;
            items_ = new_items;
            capacity_ = new_capacity;
        }
        if (size_ == 0) {
            items_ = new Type[capacity_];
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    void Expand(size_t new_capacity) {
        if (new_capacity == 0) {
            ++new_capacity;
        }
        auto new_items = new Type[new_capacity];
        std::copy(SimpleVector<Type>::Iterator(items_), SimpleVector<Type>::Iterator(items_ + size_),
                  SimpleVector<Type>::Iterator(new_items));
        delete[] items_;
        items_ = new_items;
        capacity_ = new_capacity;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type &value) {
        if (pos == items_ + size_) {
            PushBack(value);
            return items_ + size_ - 1;
        }
        if (capacity_ == size_) {
            capacity_ *= 2;
            auto new_items = new Type[capacity_];
            std::copy(SimpleVector<Type>::Iterator(items_), SimpleVector<Type>::Iterator(pos),
                      SimpleVector<Type>::Iterator(new_items));
            std::copy(SimpleVector<Type>::Iterator(pos), SimpleVector<Type>::Iterator(items_ + size_),
                      SimpleVector<Type>::Iterator(new_items + (pos - items_) + 1));
            new_items[(pos - items_)] = value;
            delete[] items_;
            items_ = new_items;
        } else {
            std::copy_backward(SimpleVector<Type>::Iterator(pos),
                               SimpleVector<Type>::Iterator(items_ + size_),
                               SimpleVector<Type>::Iterator(items_ + size_ + 1));
            items_[pos - items_] = value;
        }
        ++size_;
        return items_ + (pos - items_);
    }

    Iterator Insert(ConstIterator pos, Type &&value) {
        assert(pos >= begin() && pos <= end());
        if (pos == items_ + size_) {
            PushBack(std::move(value));
            return items_ + size_ - 1;
        }
        if (capacity_ == size_) {
            capacity_ *= 2;
            auto new_items = new Type[capacity_];
            std::move(SimpleVector<Type>::Iterator(items_), SimpleVector<Type>::Iterator(pos),
                      SimpleVector<Type>::Iterator(new_items));
            std::move(SimpleVector<Type>::Iterator(pos), SimpleVector<Type>::Iterator(items_ + size_),
                      SimpleVector<Type>::Iterator(new_items + (pos - items_) + 1));
            new_items[(pos - items_)] = std::move(value);
            delete[] items_;
            items_ = new_items;
        } else {
            std::move_backward(SimpleVector<Type>::Iterator(pos),
                               SimpleVector<Type>::Iterator(items_ + size_),
                               SimpleVector<Type>::Iterator(items_ + size_ + 1));
            items_[pos - items_] = std::move(value);
        }
        ++size_;
        return items_ + (pos - items_);
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ == 0) {
            return;
        }
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);
        std::move(begin()+dist+1,end(),begin()+dist);
        --size_;
        return items_ + dist;
    }


    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(items_, other.items_);
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ < new_capacity) {
            auto new_item = new Type[size_];
            std::copy(SimpleVector<Type>::Iterator(items_), SimpleVector<Type>::Iterator(items_ + size_), new_item);
            delete[] items_;
            items_ = new_item;
            capacity_ = new_capacity;
        }
    }


// Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        items_ = new Type[size];
        capacity_ = size;
        size_ = size;
        for (size_t i = 0; i < size; ++i) {
            items_[i] = 0;
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value) {
        items_ = new Type[size];
        capacity_ = size;
        size_ = size;
        for (size_t i = 0; i < size; ++i) {
            items_[i] = value;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        size_ = init.size();
        capacity_ = init.size();
        items_ = new Type[size_];
        std::copy(init.begin(), init.end(), items_);
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
        if (new_size > capacity_) {
            auto new_items = new Type[new_size];
            std::move(items_, items_ + size_, new_items);
            for (size_t i = size_; i < new_size; ++i) {
                new_items[i] = Type();
            }
            delete items_;
            items_ = std::move(new_items);
            capacity_ = new_size * 2;
        } else if (new_size <= capacity_) {
            auto new_items = new Type[new_size];
            std::move(items_, items_ + new_size, new_items);
            if (new_size > size_) {
                for (size_t i = size_; i < new_size; ++i) {
                    new_items[i] = Type();
                }
            }
            delete[] items_;
            items_ = std::move(new_items);
        }
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_ + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_ + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_ + size_;
    }

    ~SimpleVector() {
        delete[] items_;
    }

private:
    Iterator items_ = nullptr;

    size_t size_ = 0;
    size_t capacity_ = 0;
};


template<typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }

    for (size_t i = 0; i < lhs.GetSize(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(lhs == rhs);
}


template<typename Type>
bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return (operator>(lhs, rhs) || operator==(lhs, rhs));
}

template<typename Type>
bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}


template<typename Type>
bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return (!operator<(lhs, rhs) && !operator==(lhs, rhs));
}

template<typename Type>
bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return (operator<(lhs, rhs) || operator==(lhs, rhs));
}

