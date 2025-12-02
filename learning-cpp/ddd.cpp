#include <iostream>

class MyDynamicArray {
  private:
    int *data_;
    int size_;

    MyDynamicArray(const MyDynamicArray &other) = delete;
    MyDynamicArray &operator=(const MyDynamicArray &other) = delete;

  public:
    MyDynamicArray(int initialSize) : size_(initialSize) {
      data_ = new int[initialSize];

      for (int i = 0; i < size_; ++i) {
        data_[i] = 0;
      }
    }

    ~MyDynamicArray() {
      delete[] data_;
    }

    int getSize() const {
      return size_;
    }

    int &getElement(int index) {
      if (index < 0 || index >= size_) {
        std::cerr << "Error: Index out of bounds!" << std::endl;
        return data_[0];
      }
      return data_[index];
    }
};

int main() {
  MyDynamicArray arr(10);

  arr.getElement(3) = 42;

  std::cout << "Element at index 3: " << arr.getElement(3) << std::endl;
  std::cout << "Size: " << arr.getSize() << std::endl;

  return 0;
}