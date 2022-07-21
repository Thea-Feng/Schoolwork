#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <cstdio>
#include <bitset>
#include "LabelTable.h"
using namespace std;
map<string, string> m, op;
map<string, int> ad;
void LabelTable::init(){
    m["add"] = "100000";
    m["addu"] = "100001"; 
    m["and"] = "100100";
    m["div"] = "011010";
    m["divu"] = "011011"; 
    m["jalr"] = "001001"; 
    m["jr"] = "001000"; 
    m["mfhi"] = "010000"; 
    m["mflo"] = "010010"; 
    m["mthi"] = "010001";
    m["mtlo"] = "010011";
    m["mult"] = "011000";
    m["multu"] = "011001";
    m["nor"] = "100111";
    m["or"] = "100101";
    m["sll"] = "000000";
    m["sllv"] = "000100";
    m["slt"] = "101010";
    m["sltu"] = "101011";
    m["sra"] = "000011";
    m["srav"] = "000111";
    m["srl"] = "000010";
    m["srlv"] = "000110";
    m["sub"] = "100010";
    m["subu"] = "100011";
    m["syscall"] = "001100";
    m["xor"] = "100110";
    m["$zero"]="00000";
    m["$at"]="00001";
    m["$v0"]="00010";
    m["$v1"]="00011";
    m["$a0"]="00100";
    m["$a1"]="00101";
    m["$a2"]="00110";
    m["$a3"]="00111";
    m["$t0"]="01000";
    m["$t1"]="01001";
    m["$t2"]="01010";
    m["$t3"]="01011";
    m["$t4"]="01100";
    m["$t5"]="01101";
    m["$t6"]="01110";
    m["$t7"]="01111";
    m["$s0"]="10000";
    m["$s1"]="10001";
    m["$s2"]="10010";
    m["$s3"]="10011";
    m["$s4"]="10100";
    m["$s5"]="10101";
    m["$s6"]="10110";
    m["$s7"]="10111";
    m["$t8"]="11000";
    m["$t9"]="11001";
    m["$k0"]="11010";
    m["$k1"]="11011";
    m["$gp"]="11100";
    m["$sp"]="11101";
    m["$fp"]="11110";
    m["$ra"]="11111";





    op["addi"]= "001000";
    op["addiu"]= "001001";
    op["andi"]= "001100";
    op["beq"]= "000100";
    op["bgez"]= "000001";
    op["bgtz"]= "000111";
    op["blez"]= "000110";
    op["bltz"]= "000001";
    op["bne"]= "000101";
    op["lb"]= "100000";
    op["lbu"]= "100100";
    op["lh"]= "100001";
    op["lhu"]= "100101";
    op["lui"]= "001111";
    op["lw"]= "100011";
    op["ori"]= "001101";
    op["sb"]= "101000";
    op["slti"]= "001010";
    op["sltiu"]= "001011";
    op["sh"]= "101001";
    op["sw"]= "101011";
    op["xori"]= "001110";
    op["lwl"]= "100010";
    op["lwr"]= "100110";
    op["swl"]= "101010";
    op["swr"]= "101110";
    op["j"]="000010";
    op["jal"]="000011";
}


string LabelTable::get(string key, int pos){
    // cout<<"key and pos is "<<key<<" "<<pos<<endl;
    if(pos == 0){
        if(m.find(key) == m.end()) cout<<"NO MAP  \'"<<key<<"\'"<<endl;
        else return m[key];
    }
    if(pos == 1){
        if(op.find(key) == op.end())return "000000";
        return op[key];
    }
}
string LabelTable::dec2bin(int x, int l){
    // char ch[40];
    // if(l == 16)
    //     sprintf(ch, "%16d", a);
    // else sprinf(ch, "%", a);
    // for(i = 0; i < l; i++)
    //     if(ch[i] == ' ') ch[i] = '0';

    // return str(ch); 
    if(l == 16){
        bitset<16> bs2(x);
        return bs2.to_string();
    }
    if(l == 5){
        bitset<5> bs2(x);
        return bs2.to_string();
    }
    if(l == 26){
        bitset<26> bs2(x);
        return bs2.to_string();
    }
    
}
void LabelTable::add(string name, int relpos){
    ad[name] = relpos;

}
string LabelTable::get_addr(string key, bool rel, int now){
    if(rel == 1){
        int t = ad[key] - now - 1;
        return dec2bin(t, 16);
    }
    else{
        int t = (0x400000 + ad[key]* 4) >>2;
        return dec2bin(t, 26);
    }
}




