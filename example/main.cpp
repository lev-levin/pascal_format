#include <iostream>
#include "pascal_format.h"

using namespace std;

int main()
{
    std::string world = "World";
    int count = 10;
    cout << FormatStr("Hello %s (%d counts)!!!", FmtArgs<<world<<count).c_str() << endl;
    return 0;
}
