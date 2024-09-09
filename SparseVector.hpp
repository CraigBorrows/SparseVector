//
// Created by craig on 9/09/2024.
//

#ifndef SPARSEVECTOR_HPP_
#define SPARSEVECTOR_HPP_

#include <vector>
#include <optional>
#include <stdexcept>
#include <algorithm>
#include <iterator>

// Helper to check if T has a memory_usage() method
template<typename T, typename = void>
struct has_memory_usage : std::false_type {};

template<typename T>
struct has_memory_usage<T, std::void_t<decltype(std::declval<T>().memory_usage())>> : std::true_type {};

template<typename T>
class SparseVector {
  private:
    std::vector<T> objects;
    std::vector<std::optional<uint32_t>> indices;
    size_t max_index = 0;

    // Helper function to get memory usage of T
    static size_t get_object_memory_usage() {
        if constexpr (has_memory_usage<T>::value) {
            return T().memory_usage();
        } else {
            return sizeof(T);
        }
    }

  public:
    // Type definitions to match standard container interface
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    template<bool IsConst>
    class Iterator {
      private:
        using ContainerType = std::conditional_t<IsConst, const SparseVector, SparseVector>;
        ContainerType* container;
        size_t current_index;

        void advance_to_valid() {
            while (current_index <= container->max_index &&
                   (current_index >= container->indices.size() || !container->indices[current_index].has_value())) {
                ++current_index;
            }
        }

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(ContainerType* cont, size_t index) : container(cont), current_index(index) {
            advance_to_valid();
        }

        reference operator*() const {
            return container->objects[*container->indices[current_index]];
        }

        pointer operator->() const {
            return &(operator*());
        }

        Iterator& operator++() {
            ++current_index;
            advance_to_valid();
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const {
            return container == other.container && current_index == other.current_index;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    iterator begin() { return iterator(this, 0); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator cbegin() const { return const_iterator(this, 0); }
    iterator end() { return iterator(this, max_index + 1); }
    const_iterator end() const { return const_iterator(this, max_index + 1); }
    const_iterator cend() const { return const_iterator(this, max_index + 1); }


    // Constructors
    SparseVector() = default;
    explicit SparseVector(size_type n) : indices(n) {}

    // Element access
    T& at(size_type pos) {
        if (pos >= indices.size() || !indices[pos].has_value()) {
            throw std::out_of_range("SparseVector::at: pos (which is "
                                    + std::to_string(pos) + ") >= this->size() (which is "
                                    + std::to_string(indices.size()) + ")");
        }
        return objects[*indices[pos]];
    }

    const T& at(size_type pos) const {
        if (pos >= indices.size() || !indices[pos].has_value()) {
            throw std::out_of_range("SparseVector::at: pos (which is "
                                    + std::to_string(pos) + ") >= this->size() (which is "
                                    + std::to_string(indices.size()) + ")");
        }
        return objects[*indices[pos]];
    }

    T& operator[](size_t pos) {
        if (pos > max_index) {
            max_index = pos;
        }
        if (pos >= indices.size()) {
            indices.resize(pos + 1);
        }
        if (!indices[pos].has_value()) {
            indices[pos] = objects.size();
            objects.emplace_back();
        }
        return objects[*indices[pos]];
    }

    const T& operator[](size_t pos) const {
        if (pos >= indices.size() || !indices[pos].has_value()) {
            throw std::out_of_range("Index out of range");
        }
        return objects[*indices[pos]];
    }


    T& front() { return objects.front(); }
    const T& front() const { return objects.front(); }
    T& back() { return objects.back(); }
    const T& back() const { return objects.back(); }

    // Capacity
    bool empty() const { return objects.empty(); }
    size_type size() const { return objects.size(); }
    size_type max_size() const { return indices.max_size(); }
//    size_type capacity() const { return indices.size(); }
    size_t capacity() const { return objects.capacity(); }
    void reserve(size_type new_cap) {
        if (new_cap > max_index) {
            indices.resize(new_cap);
            max_index = new_cap - 1;
        }
        objects.reserve(std::max(new_cap, objects.size()));
    }

    void shrink_to_fit() {
        objects.shrink_to_fit();
        indices.resize(max_index + 1);
        indices.shrink_to_fit();
    }

    // Modifiers
    void clear() {
        objects.clear();
        indices.clear();
    }

    void insert(size_t pos, const T& value) {
        if (pos > max_index) {
            max_index = pos;
        }
        if (pos >= indices.size()) {
            indices.resize(pos + 1);
        }
        if (!indices[pos].has_value()) {
            indices[pos] = objects.size();
            objects.push_back(value);
        } else {
            objects[*indices[pos]] = value;
        }
    }

    void erase(size_type pos) {
        if (pos < indices.size() && indices[pos].has_value()) {
            size_type obj_index = *indices[pos];
            objects.erase(objects.begin() + obj_index);
            indices[pos] = std::nullopt;

            // Update indices for objects that have moved
            for (auto& index : indices) {
                if (index.has_value() && *index > obj_index) {
                    --(*index);
                }
            }
        }
    }

    void push_back(const T& value) {
        objects.push_back(value);
        indices.push_back(objects.size() - 1);
    }

    void pop_back() {
        if (!objects.empty()) {
            objects.pop_back();
            while (!indices.empty() && !indices.back().has_value()) {
                indices.pop_back();
            }
            if (!indices.empty()) {
                indices.back() = std::nullopt;
            }
        }
    }

    void resize(size_type count) {
        indices.resize(count);
    }

    void swap(SparseVector& other) {
        objects.swap(other.objects);
        indices.swap(other.indices);
    }

    // Lookup
    bool contains(size_type pos) const {
        return (pos < indices.size() && indices[pos].has_value());
    }

    iterator find(size_type pos) {
        if (pos < indices.size() && indices[pos].has_value()) {
            return iterator(this, pos);
        }
        return end();
    }

    const_iterator find(size_type pos) const {
        if (pos < indices.size() && indices[pos].has_value()) {
            return const_iterator(this, pos);
        }
        return end();
    }

    // Memory usage calculation
    std::pair<size_t, size_t> memory_usage() const {
        return {
            objects.capacity() * get_object_memory_usage(),
            indices.capacity() * sizeof(std::optional<uint32_t>)
        };
    }
};

#endif //SPARSEVECTOR_HPP_
