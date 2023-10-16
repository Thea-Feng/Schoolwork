
#ifndef _LabelTable_h
#define _LabelTable_h

#include <cstdlib>
#include <string>
#include <map>
#include <cstdio>
#include <bitset>
using namespace std;
class LabelTable {

public:

    LableTable();


    void init();

    string get(string key, int pos);

    string dec2bin(int x, int l);

    void add(string name, int relpos);

    string get_addr(string key, bool rel, int now);
};    

#endif




    











