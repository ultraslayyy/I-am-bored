#include <iostream>

class MyDynamicArray {
    private:
        std::unique_ptr<int[]> data_;
        int size_;

    public:
        MyDynamicArray(int initialSize) : size_(initialSize) {
            data_ = std::make_unique<int[]>(initialSize);

            for (int i = 0; i < size_; ++i) {
                data_[i] = 0;
            }
        }

        int getSize() const {
            return size_;
        }

        int& getElement(int index) {
            if (index < 0 || index >= size_) {
                std::cerr << "Error: Index out of bounds!" << std::endl;
                return data_[0];
            }
            return data_[index];
        }
};

int main() {
    MyDynamicArray arr1(5);
    arr1.getElement(2) = 100;

    {
        MyDynamicArray tempArr(3);
        tempArr.getElement(0) = 50;
    }

    std::cout << "Arr1 element at 2: " << arr1.getElement(2) << std::endl;
    return 0;
}