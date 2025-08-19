#include <iostream>
#include <climits>

using namespace std;

constexpr int CONST_MAX_LENGTH = 100;

class CSorter {
protected:
    int data[CONST_MAX_LENGTH];
    int size;

public:

    CSorter(const int input[], int n) {
        size = n;
        for (int i = 0; i < n; ++i)
            data[i] = input[i];
    }

    void print() const {
        for (int i = 0; i < size; ++i)
            cout << data[i] << " ";
        cout << endl;
    }

    void selectionSort() {
        for (int j = 0; j < size - 1; j++){
            int min = INT_MAX;
            int pos_min = -1;
            for (int i = j; i < size; i++) {
                int elem = data[i];
                if (elem < min) {
                    min = elem;
                    pos_min = i;
                }
            }
            //swap data[j] with data[pos_min]
            if (pos_min != j) {
                int aux = data[j];
                data[j] = data[pos_min];
                data[pos_min]= aux;
            }
        }
    }
    virtual void heapSort() {}

    virtual void mergeSort() {}

    void quickSort() {

    }
};


class CHeapSorter : public CSorter {
public:
    CHeapSorter(const int input[], int n): CSorter(input, n){}

    void heapSort(){
        //build the maxheap
        for (int i = size/2-1; i >=0; i--) {
            heapify(size, i);
        }

        for (int i = size -1; i > 0; i--) {
            swap(data[0],data[i]);
            heapify(i,0);
        }

    }

private:
    void heapify(int n, int i) {

        int largest = i;
        int left =  2*i + 1;
        int right = 2*i + 2;

        if (left < n && data[left] > data[largest]) {
            largest = left;
        }
        if (right < n && data[right] > data[largest]){
            largest = right;
        }
        if (largest != i) {
            swap(data[i], data[largest]);
            heapify(n,largest);
        }

    }
};


class CMergeSorter : public CSorter {
public:
    CMergeSorter(const int input[], int n): CSorter(input , n){}

    void mergeSort() override {
        mergeSortHelper(data,size);
    }
private:
    void mergeSortHelper(int array[] ,int length) {

        // int length = sizeof(array) / 4;

        if (length <= 1) {
            return;
        }

        int middle = length / 2;
        int leftArray[CONST_MAX_LENGTH];
        int rightArray[CONST_MAX_LENGTH];
        int leftSize = middle;
        int rightSize = length - middle;

        int i = 0;
        int j = 0;

        for (; i<length; i++) {
            if (i< middle) {
                leftArray[i] = array[i];
            }
            else {
                rightArray[j] = array[i];
                j++;
            }
        }
        mergeSortHelper(leftArray, leftSize);
        mergeSortHelper(rightArray, rightSize);
        merge(leftArray,leftSize,rightArray,rightSize, array);


    }

    void merge(int leftArray[],int leftSize, int rightArray[],int rightSize, int array[]) {
        int i = 0, l =0, r =0;

        while (l < leftSize && r < rightSize) {
            if (leftArray[l] < rightArray[r]) {
                array[i] = leftArray[l];
                i++;
                l++;
            }
            else {
                array[i] = rightArray[r];
                i++;
                r++;
            }
        }

        while (l < leftSize) {
            array[i] = leftArray[l];
            i++;
            l++;
        }

        while (r < rightSize) {
            array[i] = rightArray[r];
            i++;
            r++;
        }

    }

};









int main() {

    int values[] = {8,7,9,2,3,1,10,5,4,6};
    int n = size(values);


    CHeapSorter sorter(values , n);

    cout<<"Initial array:\n";
    sorter.print();

    sorter.heapSort();

    cout<<"Final array:\n";
    sorter.print();
    cout<< endl;


}