#ifndef __PUBLIC_H
#define __PUBLIC_H

#include <iostream>

using std::cout;
using std::endl;

#define LOG(str) \
    cout << __FILE__ << ":" << __LINE__ << ":" << \
    __TIMESTAMP__ << ":" << str << endl;

#endif


