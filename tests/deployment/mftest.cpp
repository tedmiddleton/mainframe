#include <iostream>
#include <mainframe/frame.hpp>

using namespace std;
using namespace mf;

int main(void)
{
    cout << "Hello mftest!\n";
    frame<int, double, short> fr;
    fr.push_back(1, 1.10, 11);
    fr.push_back(2, 1.20, 12);
    fr.push_back(3, 1.30, 13);
    fr.push_back(4, 1.40, 14);
    fr.push_back(5, 1.50, 15);
    fr.push_back(6, 1.60, 16);
    fr.push_back(7, 1.70, 17);
    fr.push_back(8, 1.80, 18);
    fr.push_back(9, 1.90, 19);
    fr.push_back(0, 1.00, 10);
    cout << fr;
    return 0;
}

