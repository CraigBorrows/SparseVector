//
// Created by craig on 9/09/2024.
//

#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include "SparseVector.hpp"

struct ObjectWithMemoryUsage {
    int data[1000];
    static size_t memory_usage() { return sizeof(ObjectWithMemoryUsage); }
};

class CustomObject {
  public:
    int id;
    std::string name;

    // Default constructor
    CustomObject() : id(0), name("default") {}

    // Parameterized constructor
    CustomObject(int id, std::string name) : id(id), name(name) {}

    // Equality operator
    bool operator==(const CustomObject& other) const {
        return id == other.id && name == other.name;
    }

    // For printing the object in tests if needed
    friend std::ostream& operator<<(std::ostream& os, const CustomObject& obj) {
        os << "CustomObject(id: " << obj.id << ", name: " << obj.name << ")";
        return os;
    }
};

void test_basic_operations() {
    std::cout << "Testing basic operations with integers and custom objects...\n";
    SparseVector<int> sv;

    // Test insertion and access with integers
    sv[5] = 50;
    sv[10] = 100;
    assert(sv[5] == 50);
    assert(sv[10] == 100);
    assert(sv.size() == 2);

    // Test iteration with integers
    int sum = 0;
    for (const auto& item : sv) {
        sum += item;
    }
    assert(sum == 150);

    // Test erase with integers
    sv.erase(5);
    assert(sv.size() == 1);
    assert(!sv.contains(5));
    assert(sv.contains(10));

    // Test overwriting an existing value with integers
    sv[10] = 200;
    assert(sv[10] == 200);

    // Test clearing the sparse vector with integers
    sv.clear();
    assert(sv.size() == 0);
    assert(!sv.contains(10));

    // Now test with CustomObject
    SparseVector<CustomObject> svCustom;

    // Insert custom objects
    svCustom[1] = CustomObject(1, "Object1");
    svCustom[3] = CustomObject(3, "Object3");

    // Test access with custom objects
    assert(svCustom[1] == CustomObject(1, "Object1"));
    assert(svCustom[3] == CustomObject(3, "Object3"));
    assert(svCustom.size() == 2);

    // Test iteration with custom objects
    for (const auto& obj : svCustom) {
        std::cout << "  "<< obj << "\n"; // Just printing to visualize
    }

    // Test erase with custom objects
    svCustom.erase(1);
    assert(svCustom.size() == 1);
    assert(!svCustom.contains(1));
    assert(svCustom.contains(3));

    // Test overwriting a custom object
    svCustom[3] = CustomObject(3, "UpdatedObject3");
    assert(svCustom[3] == CustomObject(3, "UpdatedObject3"));

    // Test insertion of large indices with custom objects
    svCustom[100000] = CustomObject(100000, "LargeObject");
    assert(svCustom[100000] == CustomObject(100000, "LargeObject"));
    assert(svCustom.size() == 2); // Since we have erased index 1

    std::cout << "Basic operations test with custom objects passed.\n\n";
}


void test_memory_usage() {
    std::cout << "Testing memory usage...\n";



    SparseVector<ObjectWithMemoryUsage> sv_obj;
    std::vector<ObjectWithMemoryUsage> v_obj;
    std::map<int, ObjectWithMemoryUsage> m_obj;

    const int NUM_ELEMENTS = 1000;
    const int MAX_INDEX = 10000;

    // Insert elements into SparseVector, vector, and map
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        int index = rand() % MAX_INDEX;

        sv_obj[index] = ObjectWithMemoryUsage();
        if (index >= v_obj.size()) v_obj.resize(index + 1);
        v_obj[index] = ObjectWithMemoryUsage();
        m_obj[index] = ObjectWithMemoryUsage();
    }


    // Memory usage for SparseVector<ObjectWithMemoryUsage>
    auto [sv_obj_objects_mem, sv_obj_indices_mem] = sv_obj.memory_usage();
    size_t sv_obj_total_mem = sv_obj_objects_mem + sv_obj_indices_mem;


    // Memory usage for std::vector<ObjectWithMemoryUsage>
    size_t v_obj_total_mem = v_obj.capacity() * sizeof(ObjectWithMemoryUsage);

    // Memory usage for std::map<int, ObjectWithMemoryUsage>
    size_t m_obj_total_mem = m_obj.size() * (sizeof(std::pair<const int, ObjectWithMemoryUsage>) + sizeof(void*) * 2); // Approximation

    // Printing results
    std::cout << "  SparseVector<ObjectWithMemoryUsage> memory: " << sv_obj_total_mem/1024 << " Kb\n";
    std::cout << "  std::vector<ObjectWithMemoryUsage> memory: " << v_obj_total_mem/1024 << " Kb\n";
    std::cout << "  std::map<int, ObjectWithMemoryUsage> memory: " << m_obj_total_mem/1024 << " Kb\n";



    assert(sv_obj_total_mem > m_obj_total_mem);
    assert(sv_obj_total_mem < v_obj_total_mem);

    std::cout << "Memory usage test passed.\n\n";
}


void test_iterator() {
    std::cout << "Testing iterator...\n";

    SparseVector<int> sv;

    // Inserting values into the SparseVector
    sv[0] = 0;
    sv[5] = 50;
    sv[10] = 100;

    std::cout << "  Inserted values:\n";
    std::cout << "    Index 0: " << sv[0] << std::endl;
    std::cout << "    Index 5: " << sv[5] << std::endl;
    std::cout << "    Index 10: " << sv[10] << std::endl;

    int expected_sum = 150;
    int actual_sum = 0;

    std::cout << "  Testing dereference (*), pre-increment (++), and post-increment (++(int)):\n";
    auto it = sv.begin();

    // Test dereferencing and the arrow operator
    std::cout << "    Dereference (*it): " << *it << std::endl;
    assert(*it == 0);

    // Test pre-increment (++it)
    ++it;
    std::cout << "    After pre-increment (++it), Dereference (*it): " << *it << std::endl;
    assert(*it == 50);

    // Test post-increment (it++)
    it++;
    std::cout << "    After post-increment (it++), Dereference (*it): " << *it << std::endl;
    assert(*it == 100);

    // Reset iterator for sum calculation
    it = sv.begin();

    std::cout << "  Iterating over SparseVector using pre-increment (++it):\n";
    for (; it != sv.end(); ++it) {
        std::cout << "    Value: " << *it << std::endl;
        actual_sum += *it;
    }

    std::cout << "  Expected sum: " << expected_sum << ", Actual sum: " << actual_sum << std::endl;
    assert(actual_sum == expected_sum);

    // Test equality and inequality operators (== and !=)
    std::cout << "  Testing equality (==) and inequality (!=) operators:\n";
    auto it1 = sv.begin();
    auto it2 = sv.begin();
    auto it3 = sv.end();

    std::cout << "    it1 == it2: " << (it1 == it2) << std::endl;
    assert(it1 == it2);

    ++it2;
    std::cout << "    it1 != it2 after incrementing it2: " << (it1 != it2) << std::endl;
    assert(it1 != it2);

    std::cout << "    it1 != it3 (it3 is end iterator): " << (it1 != it3) << std::endl;
    assert(it1 != it3);

    std::cout << "Iterator test passed.\n";
}

void test_capacity_operations() {
    std::cout << "Testing capacity operations...\n";
    SparseVector<int> sv;

    std::cout << "  Initial capacity: " << sv.capacity() << std::endl;
    assert(sv.empty());
    sv[5] = 50;
    assert(!sv.empty());


    sv[150] = 150;
    std::cout << "  Capacity after inserting at index 150: " << sv.capacity() << std::endl;
    assert(sv.capacity() == 2);

    sv.reserve(500);
    std::cout << "  Capacity after reserve(500): " << sv.capacity() << std::endl;
    assert(sv.capacity() == 500);

    // Shrink to fit should reduce capacity to match the highest index
    sv.shrink_to_fit();
    std::cout << "  Capacity after shrink_to_fit: " << sv.capacity() << std::endl;
    assert(sv.capacity() == 2);  // Capacity should shrink to exactly 151, matching the highest used index


    std::cout << "Capacity operations test passed.\n\n";
}

void test_modifier_operations() {
    std::cout << "Testing modifier operations...\n";
    SparseVector<int> sv;

    // Insert at a specific index
    std::cout << "  Inserting 50 at index 5.\n";
    sv.insert(5, 50);
    std::cout << "  Value at index 5: " << sv[5] << std::endl;
    assert(sv[5] == 50);

    // Push back a new element
    std::cout << "  Pushing back value 100.\n";
    sv.push_back(100);
    std::cout << "  Value at back: " << sv.back() << std::endl;
    assert(sv.back() == 100);

    // Pop the last element
    std::cout << "  Popping back the last element.\n";
    sv.pop_back();
    std::cout << "  Size after pop_back: " << sv.size() << std::endl;
    assert(sv.size() == 1);

    // Print the current contents
    std::cout << "  Contents after pop_back: \n";
    sv.reserve(50);
    sv.insert(5, 52);
    sv.insert(6, 55);
    sv.insert(7, 69);
    sv[8]=99;
    sv.erase(7);
    for (const auto& value : sv) {
        std::cout << "    Value: " << value << std::endl;
    }

    // Clear the vector
    std::cout << "  Clearing the SparseVector.\n";
    sv.clear();
    std::cout << "  Is empty after clear: " << std::boolalpha << sv.empty() << std::endl;
    assert(sv.empty());

    std::cout << "Modifier operations test passed.\n\n";
}

int main() {
    test_basic_operations();
    test_memory_usage();
    test_capacity_operations();
    test_modifier_operations();
    test_iterator();



    std::cout << "All tests passed successfully!\n";
    return 0;
}