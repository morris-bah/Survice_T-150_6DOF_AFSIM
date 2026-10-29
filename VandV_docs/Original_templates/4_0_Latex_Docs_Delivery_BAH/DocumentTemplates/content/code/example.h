#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

void print_vector(const vector<int>& v) {
    for (int i : v) {
        cout << i << " ";
    }
    cout << endl;
}

int add(int a, int b) {
    return a + b;
}

int main() {
    vector<int> numbers = {1, 4, 2, 8, 5, 7};
    sort(numbers.begin(), numbers.end());
    print_vector(numbers);

    int result = add(5, 3);
    cout << "Sum: " << result << endl;

    cout << "Square root of 16 is " << sqrt(16) << endl;
    
    return 0;
}
