
#include<cstring>
#include<iostream>
#include<fstream>
#include<cstdio>
#include<ctype.h>
#include "LabelTable.h"
#include<cstdlib>
using namespace std;
const int MAXN = 10000;
int valid_line, addr;
bool is_mip[MAXN];
LabelTable Label;
void Trim(string & str)
{
    string blanks("\f\v\r\t\n ");
    str.erase(0,str.find_first_not_of(blanks));
    str.erase(str.find_last_not_of(blanks) + 1);
}

string get_opcode(string line){
    int st = line.find(':')+1, ed = line.find('#');
    if(ed != -1 && st > ed) st = 0;
    while(st < line.length() && !isalpha(line[st])) st++;
    ed = st;
    while(ed < line.length() && isalpha(line[ed])) ed++;
    // cout<<"&"<<line[st]<<"&"<<st<<endl;
    return Label.get(line.substr(st, ed-st), 1);

}
string get_R(string line){
    int st = line.find(':')+1, ed = line.find('#');
    if(ed != -1 && st > ed) st = 0;
    while(st < line.length() && !isalpha(line[st])) st++;
    ed = st;
    while(ed < line.length() && isalpha(line[ed])) ed++;

    string rs = "00000", rt = "00000", rd = "00000", sa = "00000";
    string s1 = "00000", s2 = "00000", s3 = "00000";
    string commd = line.substr(st, ed-st);
    string funct = Label.get(commd, 0);
    // cout<<line<<"      "<<commd<<endl;
    if(commd != "syscall"){
        st = ed;
        while(line[st] != '$') st++;
        ed = st + 1;
        while(ed < line.length() && (isalpha(line[ed])) || isdigit(line[ed])) ed++;
        s1 = Label.get(line.substr(st, ed-st), 0);
        if(commd == "mfhi" || commd == "mflo") rd = s1;
        else if (commd == "mthi" || commd == "mtlo" || commd == "jr") rs = s1;
        else {
            st = ed;
            while(line[st] != '$') st++;
            ed = st + 1;
            while(ed < line.length() && (isalpha(line[ed])) || isdigit(line[ed])) ed++;
            s2 = Label.get(line.substr(st, ed-st), 0);
            if(commd == "div" || commd == "divu" || commd == "mult" || commd == "multu") rs = s1, rt = s2;
            else if(commd == "jalr") rd = s1, rs = s2;
            else if(commd == "sll" || commd == "sra" || commd == "srl") {
                rd = s1, rt = s2;
                st = ed;
                while(line[st] != '-' && (line[st] < '0' || line[st] > '9')) st++;
                ed = st + 1;
                while(ed < line.length() && (line[ed] >= '0' && line[ed] <= '9')) ed++;
                const char*p = line.substr(st, ed-st).data();
                int x = atoi(p);
                sa = Label.dec2bin(x, 5);
            }
            else {
                st = ed;
                while(line[st] != '$') st++;
                ed = st + 1;
                while(ed < line.length() && (isalpha(line[ed])) || isdigit(line[ed])) ed++;
                s3 = Label.get(line.substr(st, ed-st), 0);
                
                if(commd == "sllv" || commd == "srav" || commd == "srlv") rd = s1, rt = s2, rs = s3;
                else rd = s1, rs = s2, rt = s3;
            }
        }
    }

    string ret = rs + rt + rd + sa + funct;
    return ret;

}
string get_I(string line, int now){
    int st = line.find(':')+1, ed = line.find('#');
    if(ed != -1 && st > ed) st = 0;
    while(st < line.length() && !isalpha(line[st])) st++;
    ed = st;
    while(ed < line.length() && isalpha(line[ed])) ed++;
    string commd = line.substr(st, ed-st);

    string rs = "00000", rt = "00000", immediate = "0000000000000000";
    string s1 = "00000", s2 = "00000";
    while(st < line.length() && line[st] != '$') st++;
    ed = st + 1;
    while(ed < line.length() && (isalpha(line[ed])) || isdigit(line[ed])) ed++;
    s1 = Label.get(line.substr(st, ed-st), 0);
    if(commd == "bgez" || commd == "bgtz" || commd == "blez" || commd == "bltz") {
        rs = s1;
        if(commd == "bgez") rt = "00001";
        st = ed;
        while(st < line.length() && (line[st] == ' ' || line[st] == '\t' || line[st] == ',')) st++;
        ed = st;
        while(ed < line.length() && (line[ed] != ' ' && line[ed] != '\t' && line[ed] != '#')) ed++;
        immediate = Label.get_addr(line.substr(st, ed-st), 1, now);
        
    }
    else if(commd == "beq" || commd == "bne"){
        // cout<<"here"<<endl;
        st = ed;
        while(st < line.length() && line[st] != '$') st++;
        ed = st + 1;
        while(ed < line.length() && (isalpha(line[ed])) || isdigit(line[ed])) ed++;
        s2 = Label.get(line.substr(st, ed-st), 0);

        st = ed;
        while(st < line.length() && (line[st] == ' ' || line[st] == '\t' || line[st] == ',')) st++;
        ed = st;
        while(ed < line.length() && (line[ed] != ' ' && line[ed] != '\t' && line[ed] != '#')) ed++;
        // cout<<line.substr(st, ed-st)<<endl;
        immediate = Label.get_addr(line.substr(st, ed-st), 1, now);

        rs = s1, rt = s2;

    }
    else if(commd == "ori" || commd == "xori" || commd == "addi" || commd == "addiu" || commd == "andi" || commd == "slti" || commd == "sltiu"){
        st = ed;
        while(st < line.length() && line[st] != '$') st++;
        ed = st + 1;
        while(ed < line.length() && (isalpha(line[ed])) || isdigit(line[ed])) ed++;
        s2 = Label.get(line.substr(st, ed-st), 0);

        st = ed;
        while(line[st] != '-' && (line[st] < '0' || line[st] > '9')) st++;
        ed = st + 1;
        while(ed < line.length() && (line[ed] >= '0' && line[ed] <= '9')) ed++;
        const char*p = line.substr(st, ed-st).data();
        int x = atoi(p);
        immediate = Label.dec2bin(x, 16);

        rt = s1, rs = s2;
    }
    else {
        st = ed;
        while(line[st] != '-' && (line[st] < '0' || line[st] > '9')) st++;
        ed = st + 1;
        while(ed < line.length() && (line[ed] >= '0' && line[ed] <= '9')) ed++;
        const char*p = line.substr(st, ed-st).data();
        int x = atoi(p);
        immediate = Label.dec2bin(x, 16);
        if(commd == "lui") rt = s1; 
        else {
            st = ed + 1;
            while(st < line.length() && line[st] != '$') st++;
            ed = st + 1;
            while(ed < line.length() && (isalpha(line[ed])) || isdigit(line[ed])) ed++;
            s2 = Label.get(line.substr(st, ed-st), 0);

            rt = s1, rs = s2;
        }
        
    }


    string ret = rs + rt + immediate;
    return ret;

}
string get_J(string line){
    int st = line.find(':')+1, ed = line.find('#');
    if(ed != -1 && st > ed) st = 0;
    while(st < line.length() && !isalpha(line[st])) st++;
    ed = st;
    while(ed < line.length() && isalpha(line[ed])) ed++;
    
    st = ed;
    while(st < line.length() && (line[st] == ' ' || line[st] == '\t')) st++;
    ed = st;
    while(ed < line.length() && (line[ed] != ' ' && line[ed] != '\t' && line[ed] != '#')) ed++;
    string address = Label.get_addr(line.substr(st, ed-st), 0, 0);
    return address;
}
void Phase1(char * file_name){
    ifstream fin (file_name);
    // ofstream fout ("incpy.txt");
    Label.init();
    string line;
    bool beg = 0;
    addr = 0x400000;
    valid_line = -1;
    memset(is_mip, 0, sizeof(0));
    int cnt = -1;
    while(!fin.eof()){
        // getline(cin, line);
        getline(fin, line);
        Trim(line);
        cnt += 1;
        int st = 0, ed = 0;
        if(line.length() == 0)continue;
        ed = line.find("#");
        if(ed == -1)ed = line.length();
        while(st < ed && (line[st] == ' ' || line[st] == '\t'))st++;
        if(st == ed)continue;
        if(line.find(".text") != -1 && line.find(".text") < ed){beg = 1; continue;}
        if(!beg) continue;
        valid_line = max(valid_line, 0);
        int pos = line.find(":");
        if( pos == -1 || pos > ed){
            valid_line += 1;
            is_mip[cnt] = 1;
        }
        else{
            Label.add(line.substr(st, pos - st), valid_line);
            st = ed = pos + 1;
            ed = line.find("#", st);
            if(ed == -1)ed = line.length();
            while(st < ed && !isalpha(line[st]))st++;
            if(st == ed)continue;
            valid_line += 1;
            is_mip[cnt] = 1;
        }
        
    }
    // for(int i = 0; i < cnt; i++)
    //     cout<<i<<" "<<is_mip[i]<<endl;
    // cout<<"Phase 1 done"<<endl;

}

void Phase2(char * input_file, char * output_file){
    // cout<<"PHase 2 beg"<<endl;
    // ifstream fin ("incpy.txt");
    ifstream fin (input_file);
    ofstream fout (output_file);
    string line;
    int cnt = -1, now = -1;

    while(!fin.eof()){
        getline(fin, line);
        cnt += 1;
        if(!is_mip[cnt])continue;
        Trim(line);
        // cout<<cnt<<endl;
        now += 1;
        string opcode = get_opcode(line);
        string ret = "";
        // cout<<"opcode of "<<line<<" is "<<opcode<<endl;
        fout<<opcode;
        if(opcode == "000000") 
            ret = get_R(line);
        else if (opcode == "000010" || opcode == "000011")
            ret = get_J(line);
        else 
            ret = get_I(line, now);
        
        fout<<ret<<endl;

        
    }
    // cout<<"Phase 2 done"<<endl;
}

int compare_files(char * file1, char * file2)
{
	ifstream op;
    string str1,str2;
    op.open(file1);
    while(!op.eof()){
        getline(op, str1);
        Trim(str1);
        str1+=str1;

    }
    op.close();
    op.open(file2);
    while(!op.eof()){
        getline(op, str2);
        Trim(str1);
        str1+=str1;

    }
    op.close();
	return 0;
}
int main (int argc, char * argv[])
{   
    if(argc < 4)
    {
        printf("Please enter an input file, an output file, and expected output file \n");
    }
    // LabelTable table;
    // table = pass1(argv[1]);
    // (void)pass2(argv[1], table);

    Phase1(argv[1]);
    Phase2(argv[1], argv[2]);

    // cout<<"Phase2 stop"<<endl;
    FILE* fp1;
    FILE* fp2;
    fp1 = fopen(argv[3], "r");
    fp2 = fopen(argv[2], "r");

    if(fp1 == NULL || fp2 == NULL){
    	printf("Error: Files are not open correctly \n");
    }

    int res = compare_files(argv[2], argv[3]);

    if(res == 0){
    	printf("ALL PASSED! CONGRATS :) \n");
    }else{
    	printf("YOU DID SOMETHING WRONG :( \n");
    }

    return 0;
}



