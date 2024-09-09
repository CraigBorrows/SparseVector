#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <random>
#include <algorithm>
#include <optional>
#include <cstdint>
#include <iomanip>
#include "SparseVector.hpp"

struct LargeObject {
    int id;
    std::vector<double> data;

    LargeObject(int i = 0) : id(i), data(1000, i) {}  // 1000 doubles

    static size_t memory_usage() {
        return sizeof(int) + (sizeof(double) * 1000) + sizeof(std::vector<double>);
    }
};

template<typename T>
size_t get_memory_usage(const T& container) {
    if constexpr (std::is_same<T, std::vector<std::optional<LargeObject>>>::value) {
        return container.capacity() * sizeof(std::optional<LargeObject>) +
               container.size() * LargeObject::memory_usage();
    } else if constexpr (std::is_same<T, SparseVector<LargeObject>>::value) {
        auto [objects_mem, indices_mem] = container.memory_usage();
        return objects_mem + indices_mem;
    } else {
        // For map and unordered_map, this is an approximation
        return container.size() * (sizeof(int) + LargeObject::memory_usage());
    }
}



template<typename T>
void print_detailed_memory_usage(const std::string& name, const T& container) {
    std::cout << std::fixed << std::setprecision(2);
    if constexpr (std::is_same<T, std::vector<std::optional<LargeObject>>>::value) {
        size_t total_optionals = container.capacity() * sizeof(std::optional<LargeObject>);
        size_t total_objects = container.size() * LargeObject::memory_usage();
        size_t object_count = std::count_if(container.begin(), container.end(),
                                            [](const auto& opt) { return opt.has_value(); });
        std::cout << name << " detailed memory usage:\n"
                  << "  Vector capacity: " << container.capacity() << " elements\n"
                  << "  Vector size: " << container.size() << " elements\n"
                  << "  Total optional<LargeObject> size: " << total_optionals / 1024.0 << " KB\n"
                  << "  Number of actual LargeObjects: " << object_count << "\n"
                  << "  Total LargeObject size: " << total_objects / 1024.0 << " KB\n"
                  << "  Total memory usage: " << (total_optionals + total_objects) / 1024.0 << " KB\n\n";
    } else if constexpr (std::is_same<T, SparseVector<LargeObject>>::value) {
        auto [objects_mem, indices_mem] = container.memory_usage();
        std::cout << name << " detailed memory usage:\n"
                  << "  Objects vector size: " << objects_mem / 1024.0 << " KB\n"
                  << "  Indices vector size: " << indices_mem / 1024.0 << " KB\n"
                  << "  Total memory usage: " << (objects_mem + indices_mem) / 1024.0 << " KB\n\n";
    } else {
        std::cout << name << " memory usage: " << get_memory_usage(container) / 1024.0 << " KB\n\n";
    }
}

template<typename T>
void addObjects(T& container, const std::vector<int>& ids) {
    for (int id : ids) {
        if constexpr (std::is_same<T, std::vector<std::optional<LargeObject>>>::value) {
            if (id >= container.size()) {
                container.resize(id + 1);
            }
            container[id].emplace(id);
        } else {
            container[id] = LargeObject(id);
        }
    }
}

template<typename T>
void readObjects(const T& container, const std::vector<int>& ids) {
    for (int id : ids) {
        if constexpr (std::is_same<T, std::vector<std::optional<LargeObject>>>::value) {
            if (id < container.size() && container[id].has_value()) {
                volatile auto temp = container[id]->id;
            }
        } else {
            auto it = container.find(id);
            if (it != container.end()) {
                volatile auto temp = it->second.id;
            }
        }
    }
}

template<>
void readObjects(const SparseVector<LargeObject>& container, const std::vector<int>& ids) {
    for (int id : ids) {
        auto it = container.find(id);
        if (it != container.end()) {
            volatile auto temp = it->id;
        }
    }
}




template<typename T>
void runTest(const std::string& name, T& container, const std::vector<int>& ids) {
    auto startAdd = std::chrono::high_resolution_clock::now();
    addObjects(container, ids);
    auto endAdd = std::chrono::high_resolution_clock::now();

    auto startRead = std::chrono::high_resolution_clock::now();
    readObjects(container, ids);
    auto endRead = std::chrono::high_resolution_clock::now();

    auto addTime = std::chrono::duration_cast<std::chrono::microseconds>(endAdd - startAdd).count();
    auto readTime = std::chrono::duration_cast<std::chrono::microseconds>(endRead - startRead).count();

    std::cout << name << " add time: " << addTime << " microseconds\n";
    std::cout << name << " read time: " << readTime << " microseconds\n";
    std::cout << name << " size: " << container.size() << " elements\n";
    print_detailed_memory_usage(name, container);
}


std::vector<int> generateUniqueRandomIDs(int count, int maxID) {
    std::vector<int> ids(maxID);
    std::iota(ids.begin(), ids.end(), 1);
    std::shuffle(ids.begin(), ids.end(), std::mt19937{std::random_device{}()});
    ids.resize(count);
    std::sort(ids.begin(), ids.end());
    return ids;
}

int main() {
    const int objectCount = 1000;
    const int maxID = 10000;

    std::vector<int> ids = generateUniqueRandomIDs(objectCount, maxID);

    std::vector<std::optional<LargeObject>> vec(maxID);
    std::map<int, LargeObject> map;
    std::unordered_map<int, LargeObject> umap;
    SparseVector<LargeObject> svec;

    runTest("Vector", vec, ids);
    runTest("Map", map, ids);
    runTest("Unordered Map", umap, ids);
    runTest("Sparse Vector", svec, ids);

    return 0;
}