#pragma once
#include "array_ptr.h"
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity_to_reserve) : capacity_(capacity_to_reserve) {
    }
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
private:
    size_t capacity_ = 0;
};

inline ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(ReserveProxyObj reserve_obj) : items_(reserve_obj.GetCapacity()), size_(0),
                                                capacity_(reserve_obj.GetCapacity()) {
    }

    explicit SimpleVector(size_t size) : SimpleVector(size, Type{}) {
    }

    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size) {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }

    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) : items_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector(SimpleVector&& other) noexcept {
        swap(other);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            swap(rhs);
        }
        return *this;
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size <= capacity_) {
            std::fill(items_.Get() + size_, items_.Get() + new_size, Type{});
            size_ = new_size;
            return;
        }
        size_t new_capacity = std::max(new_size, capacity_ * 2);
        ArrayPtr<Type> new_items(new_capacity);
        std::move(items_.Get(), items_.Get() + size_, new_items.Get());
        std::fill(new_items.Get() + size_, new_items.Get() + new_size, Type{});
        items_.swap(new_items);
        size_ = new_size;
        capacity_ = new_capacity;
    }

    void PushBack(const Type& value) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        items_[size_] = value;
        ++size_;
    }

    void PushBack(Type&& value) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        items_[size_] = std::move(value);
        ++size_;
    }

    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t index = static_cast<size_t>(pos - cbegin());
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);
            std::move(items_.Get(), items_.Get() + index, new_items.Get());
            new_items[index] = value;
            std::move(items_.Get() + index, items_.Get() + size_, new_items.Get() + index + 1);
            items_.swap(new_items);
            capacity_ = new_capacity;
        } else {
            std::move_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[index] = value;
        }
        ++size_;
        return begin() + index;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t index = static_cast<size_t>(pos - cbegin());
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_items(new_capacity);
            std::move(items_.Get(), items_.Get() + index, new_items.Get());
            new_items[index] = std::move(value);
            std::move(items_.Get() + index, items_.Get() + size_, new_items.Get() + index + 1);
            items_.swap(new_items);
            capacity_ = new_capacity;
        } else {
            std::move_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[index] = std::move(value);
        }
        ++size_;
        return begin() + index;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos < cend());
        size_t index = static_cast<size_t>(pos - cbegin());
        std::move(items_.Get() + index + 1, items_.Get() + size_, items_.Get() + index);
        --size_;
        return begin() + index;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        ArrayPtr<Type> new_items(new_capacity);
        std::move(items_.Get(), items_.Get() + size_, new_items.Get());
        items_.swap(new_items);
        capacity_ = new_capacity;
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
