#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <functional>
#include <set>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <fstream>
// #include <stdexcept>
using namespace std;
set<int> checkpoints;



int32_t im2int(const string &str){
    int32_t val = stoi(str, nullptr, 2);
    if(str[0] == '1') val |= (((1<<16)-1) << 16);
    return val;
}
int32_t extend(int32_t now){
    string s = bitset<32>(now).to_string();
    int i = 0;
    while(s[i] != '1') s[i++] = '1';
    int32_t ret = stoi(s, nullptr, 2);
    return ret;
}
const int32_t maxn = 0b1111111111111111111111111111111;
class Assembler
{
public:    
    inline static vector<string> comb;
    
    void prepocess(istream &in);
    void combine(istream &in);
    void assemble(istream &in1, istream &in2);
    
    Assembler(){};  
};

void Assembler::assemble(istream &in1, istream &in2){
    prepocess(in1);
    combine(in2);
}
void print(vector<string> out){
    for(int i = 0; i < out.size(); i++)
        cout<<out[i]<<endl;
}
string dec2bin(string s, int len){
    int32_t x = stoi(s, nullptr, 10);
    if(len == 16) return (bitset<16>(x).to_string());
    else if(len == 8)return (bitset<8>(x).to_string());
    // else if(len == 8)return (bitset<16>(x).to_string());
    else if(len == 32) return (bitset<32>(x).to_string());
    // else if(len == 26) return (bitset<26>(x).to_string());
}
string ascii2bin(string s){
    string ret = "";
    for(int i = 0; i < s.size(); i++){
        if(s[i] == '\\' && i+1 < s.size()){
            i += 1;
            char c = s[i];
            if(c == 'n') ret = bitset<8>('\n').to_string() + ret;
            else if(c == 't') ret = bitset<8>('\t').to_string() + ret;
            else if(c == 'r') ret = bitset<8>('\r').to_string() + ret;
            else if(c == '\'') ret = bitset<8>('\'').to_string() + ret;
            else if(c == '\"') ret = bitset<8>('\"').to_string() + ret;
            else if(c == '\\') ret = bitset<8>('\\').to_string() + ret;
        }
        else ret = bitset<8>(s[i]).to_string() + ret;
    }
    return ret;
}
void Assembler::prepocess(istream &in){
    string s;
    vector<string> tmp, now;
    bool read = false;
    while(getline(in, s)){
        if(s.find(".data") != string::npos) read = true;
        if(s.find(".text") != string::npos) read = false;
        if(read && s.find(".data") == string::npos) tmp.push_back(s);
    }
    // print(tmp)
    for(int i = 0; i < tmp.size(); i++){
        string s = tmp[i];
        // remove comment
        if(s.find('#') != -1) s.erase(s.find('#'));
        // remove empty lines
        if(s.find_first_not_of(' ') == string::npos) continue;
        if(!s.empty()) now.push_back(s);
    }

    comb.push_back(".data");

    string prefix = "00000000";
    for(int i = 0; i < now.size(); i++){
        string s = now[i];
        string str_type, str="";
        int st, ed;
        if(s.find(".asciiz") != string::npos){
            str_type = ".asciiz";
            st = s.find(str_type) + str_type.size();
            st = s.find('\"', st) + 1;
            ed = s.find('\"', st);
            // add '\0'
            str = prefix + ascii2bin(s.substr(st, ed - st));
            comb.push_back(str);
        }
        else if(s.find(".ascii") != string::npos){
            str_type = ".ascii";
            st = s.find(str_type) + str_type.size();
            st = s.find('\"', st) + 1;
            ed = s.find('\"', st);
            str = ascii2bin(s.substr(st, ed - st));
            comb.push_back(str);
        }
        else if(s.find(".word") != string::npos){
            str_type = ".word";
            st = s.find(str_type) + str_type.size();
            while(st < s.size()){
                while(st < s.size() && (s[st] < '0' || s[st] > '9') && s[st] != '-')st++;
                if(st == s.size()) break;
                ed = st;
                while(ed < s.size() && s[ed] >= '0' && s[ed] <= '9') ed++;
                str += dec2bin(s.substr(st, ed - st), 32) ;
                st = ed;
            }
            
            comb.push_back(str);

        }
        else if(s.find(".byte") != string::npos){
            str_type = ".byte";
            vector<string>tmp;
            st = s.find(str_type) + str_type.size();
            while(st < s.size()){
                while(st < s.size() && (s[st] < '0' || s[st] > '9') && s[st] != '-')st++;
                if(st == s.size()) break;
                ed = st;
                while(ed < s.size() && s[ed] >= '0' && s[ed] <= '9') ed++;
                tmp.push_back( dec2bin(s.substr(st, ed - st), 8) );
                st = ed;
            }
            for(int j = tmp.size()-1; j >=0; j--)
                str += tmp[j];
            comb.push_back(str);

        }
        else if(s.find(".half") != string::npos){
            str_type = ".half";
            vector<string>tmp;
            st = s.find(str_type) + str_type.size();
            while(st < s.size()){
                while(st < s.size() && (s[st] < '0' || s[st] > '9') && s[st] != '-')st++;
                if(st == s.size()) break;
                ed = st;
                while(ed < s.size() && s[ed] >= '0' && s[ed] <= '9') ed++;
                tmp.push_back( dec2bin(s.substr(st, ed - st), 16) );
                st = ed;
            }
            for(int j = tmp.size()-1; j >=0; j--)
                str += tmp[j];
            comb.push_back(str);

        }
    }


}
void Assembler::combine(istream &in){
    for(int i = 1; i < comb.size(); i++){
        string s = comb[i];
        while(s.size() > 32){
            comb.insert(comb.begin() + i, s.substr(s.size() - 32, 32));
            i++;
            s = s.substr(0, s.size() - 32);
            if(!s.empty() && s.size() <= 32)
                comb[i] = s;
        }
        if(comb[i].size() < 32)
            comb[i] = string(32 - comb[i].size(), '0') + comb[i];

    }


    comb.push_back(".text");
    string s;
    while(getline(in, s)){
        comb.push_back(s);
    }
    // print(comb);

}
class Simulator{
public:

    const vector<string> &input;
    istream &simin;
    ostream &simout;
    char* simcheck;
    Simulator(vector<string> &comb, char* check_file, istream &in_file, ostream &out_file)
        : input(comb), simcheck(check_file), simin(in_file), simout(out_file) {}

    inline static map <string, int> reg_idx;
    inline static int32_t reg[40];

    static const int v0 = 2;
    static const int a0 = 4;
    static const int a1 = 5;
    static const int a2 = 6;
    static const int sp = 29, fp = 30, gp = 28;
    static const int lo = 32;
    static const int hi = 33;
    static const int reg_tot = 34;
    static const int memory_tot = 6*1024*1024;
    static const uint32_t stk_ed = memory_tot, static_st = 1024*1024;
    uint32_t dyn_ed, text_ed = 0, static_ed = static_st;
    static const int mem_base = 0x400000;

    // static const int v0 = 2;
    // static const int v0 = 2;
    // static const int v0 = 2;
    static const int64_t twop32 = 4294967296;
    static const int word_size = 32, byte_size = 8, half_size = 16;
    inline static unsigned char memory[memory_tot+1][9];
    uint32_t pc;
    void store(string unit, uint32_t addr, string type);
    string load(uint32_t addr, string type);   
    int get_idx(uint32_t addr);   
    uint32_t get_addr(int idx); 
    int32_t load_word_val(uint32_t addr);
    int16_t load_half_val(uint32_t addr);
    int8_t load_byte_val(uint32_t addr);
    void simulate();
    void init();
    void excuate(const string &instr);
    static void throw_exception(const string &err){
        throw invalid_argument(err);
    }
    void init_checkpoints(char* checkpoint_file);
    void checkpoint_memory(int ins_count);
    void checkpoint_register(int ins_count);
    int32_t &get_reg(const string &reg_str);

    
// R-type
    void MIPS_add(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = get_reg(rs) + get_reg(rt);
    }
    void MIPS_addu(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = get_reg(rs) + get_reg(rt);
    }
    void MIPS_sub(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = get_reg(rs) - get_reg(rt);
    }
    void MIPS_subu(const string &rd, const string &rs, const string &rt){
        // here is an pro
        get_reg(rd) = (uint32_t)get_reg(rs) - (uint32_t)get_reg(rt);
    }
    void MIPS_and(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = get_reg(rs) & get_reg(rt);
    }
    void MIPS_div(const string &rs, const string &rt){
        if(reg[reg_idx[rt]] == 0) {cout<<"Divide 0"<<endl; return;}
        reg[lo] = get_reg(rs) / get_reg(rt);
        reg[hi] = get_reg(rs) % get_reg(rt);
    }
    void MIPS_divu(const string &rs, const string &rt){
        if(reg[reg_idx[rt]] == 0) {cout<<"Divide 0"<<endl; return;}
        reg[lo] = (uint32_t)get_reg(rs) / (uint32_t)get_reg(rt);
        reg[hi] = (uint32_t)get_reg(rs) % (uint32_t)get_reg(rt);
    }
    void MIPS_mult(const string &rs, const string &rt){
        int64_t tmp = (int64_t)get_reg(rs) * (int64_t)get_reg(rt);
        reg[lo] = tmp;
        reg[hi] = tmp >> 32;
        // just levea low bit?
    }
    void MIPS_multu(const string &rs, const string &rt){
        uint64_t tmp = (uint64_t)get_reg(rs) * (uint64_t)get_reg(rt);
        reg[lo] = tmp;
        reg[hi] = tmp >> 32;
    }
    void MIPS_nor(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = ~(get_reg(rs) | get_reg(rt));
    }
    void MIPS_or(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = get_reg(rs) | get_reg(rt);
    }
    void MIPS_sll(const string &rd, const string &rt, const string &sa){
        int32_t val = stoi(sa, nullptr, 2);
        get_reg(rd) = get_reg(rt) << val;
    }
    void MIPS_srl(const string &rd, const string &rt, const string &sa){
        int32_t val = stoi(sa, nullptr, 2);
        get_reg(rd) = get_reg(rt) >> val;     
    }
    void MIPS_sllv(const string &rd, const string &rt, const string &rs){
        int32_t val = get_reg(rs) & (0b1111);
        get_reg(rd) = get_reg(rt) << val;
    }
    void MIPS_sra(const string &rd, const string &rt, const string &sa){
        int32_t val = stoi(sa, nullptr, 2);
        int sign = get_reg(rt) >> 31; 
        int32_t logic_res = get_reg(rt) >> val;
        int32_t ret = logic_res;
        if(sign) ret = extend(logic_res);
        get_reg(rd) = ret;
    }
    void MIPS_srav(const string &rd, const string &rt, const string &rs){
        int32_t val = get_reg(rs) & (0b1111);
        int sign = get_reg(rt) >> 31; 
        int32_t logic_res = get_reg(rt) >> val;
        int32_t ret = logic_res;
        if(sign) ret = extend(logic_res);
        get_reg(rd) = ret;
    }
    
    void MIPS_srlv(const string &rd, const string &rt, const string &rs){
        int32_t val = get_reg(rs) & (0b1111);
        get_reg(rd) = get_reg(rt) >> val;
    }
    void MIPS_xor(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = get_reg(rs) ^ get_reg(rt);
    }
    void MIPS_slt(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = (get_reg(rs) < get_reg(rt)) ? 1 : 0;
    }
    void MIPS_sltu(const string &rd, const string &rs, const string &rt){
        get_reg(rd) = ((uint32_t)get_reg(rs) < (uint32_t)get_reg(rt)) ? 1 : 0;
    }
    void MIPS_jalr(const string &rd, const string &rs){
        get_reg(rd) = pc;
        pc = get_reg(rs);
    }
    void MIPS_jr(const string &rs){
        pc = get_reg(rs);
    }
    void MIPS_mfhi(const string &rd){
        get_reg(rd) = reg[hi];
    }
    void MIPS_mflo(const string &rd){
        get_reg(rd) = reg[lo];
    }
    void MIPS_mthi(const string &rs){
        reg[hi] = get_reg(rs);
    }
    void MIPS_mtlo(const string &rs){
        reg[lo] = get_reg(rs);
    }

// 27 - syscall
//  I-type
    
    void MIPS_addi(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = get_reg(rs) + val;
    }
    void MIPS_addiu(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = get_reg(rs) + val;
    }
    void MIPS_andi(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = get_reg(rs) & val;
    }
    void MIPS_ori(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = get_reg(rs) | val;
    }
    void MIPS_xori(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = get_reg(rs) ^ val;
    }
    void MIPS_lui(const string &rt, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = val << 16;
    }
    void MIPS_slti(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = (get_reg(rs) < val) ? 1 : 0;
    }
    void MIPS_sltiu(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        get_reg(rt) = ((uint32_t)get_reg(rs) < (uint32_t)val) ? 1 : 0;
    }
    void MIPS_beq(const string &rs, const string &rt, const string &im){
        int32_t val = im2int(im);
        val <<= 2;
        if(get_reg(rt) == get_reg(rs)) 
            pc += val;
    }
    void MIPS_bne(const string &rs, const string &rt, const string &im){
        int32_t val = im2int(im);
        val <<= 2;
        if(get_reg(rt) != get_reg(rs)) 
            pc += val;
    }
    void MIPS_bgez(const string &rs, const string &im){
        int32_t val = im2int(im);
        val <<= 2;
        if(get_reg(rs) >= 0)
            pc += val;
    }
    void MIPS_bgtz(const string &rs, const string &im){
        int32_t val = im2int(im);
        val <<= 2;
        if(get_reg(rs) > 0)
            pc += val;
    }
    void MIPS_blez(const string &rs, const string &im){
        int32_t val = im2int(im);
        val <<= 2;
        if(get_reg(rs) <= 0)
            pc += val;
    }
    void MIPS_bltz(const string &rs, const string &im){
        int32_t val = im2int(im);
        val <<= 2;
        if(get_reg(rs) < 0)
            pc += val;
    }
    void MIPS_lb(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        int8_t byte = load_byte_val(get_reg(rs) + val);
        int fg = byte>>7;
        get_reg(rt) = fg ? (((1<<24) - 1) << 8) | byte : byte;
    }
    void MIPS_lbu(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        int8_t byte = load_byte_val(get_reg(rs) + val);
        // get_reg(rt) = byte & numeric_limits<int8_t>::max();
        get_reg(rt) = byte;
    }
    void MIPS_lh(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        int16_t half = load_half_val(get_reg(rs) + val);
        int fg = half>>15;
        get_reg(rt) = fg ? (((1<<16) - 1) << 16) | half: half;
    }
    void MIPS_lhu(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        int16_t half = load_half_val(get_reg(rs) + val);
        // get_reg(rt) = half & numeric_limits<int16_t>::max();
        get_reg(rt) = half;

    }
    void MIPS_lw(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        int32_t word = load_word_val(get_reg(rs) + val);
        get_reg(rt) = word;
    }
    void MIPS_lwl(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        uint32_t addr = get_reg(rs) + val;
        int32_t mem_val = load_word_val(addr);
        int32_t reg_val = reg[reg_idx[rt]];
        int ed = addr & 3;
         
        if(ed == 0) {
            int32_t tmp = mem_val;
            get_reg(rt) = tmp;
        }
        if(ed == 1) {
            int32_t tmp = ((mem_val >> 8) << 8) + (reg_val & ((1<<8)-1));
            get_reg(rt) = tmp;
        }
        if(ed == 2) {
            int32_t tmp = ((mem_val >> 16) << 16) + (reg_val & ((1<<16)-1));
            get_reg(rt) = tmp;
        }
        if(ed == 3) {
            int32_t tmp = ((mem_val >> 24) << 24) + (reg_val & ((1<<24)-1));
            get_reg(rt) = tmp;
        }
    }
    void MIPS_lwr(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        uint32_t addr = get_reg(rs) + val;
        int32_t mem_val = reg[reg_idx[rs]];
        int32_t reg_val = reg[reg_idx[rt]];
        int ed = addr & 3;
         
        if(ed == 0) {
            int32_t tmp = (mem_val >> 24) + ((reg_val >> 8) << 8);
            get_reg(rt) = tmp;
        }

        if(ed == 1) {
            int32_t tmp = (mem_val >> 16) + ((reg_val >> 16) << 16);
            get_reg(rt) = tmp;
        }
        if(ed == 2) {
            int32_t tmp = (mem_val >> 8) + ((reg_val >> 24) << 24);
            get_reg(rt) = tmp;
        }
        if(ed == 3) {
            int32_t tmp = mem_val;
            get_reg(rt) = tmp;
        }
    }
    void MIPS_swl(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        uint32_t addr = get_reg(rs) + val;
        int32_t mem_val = reg[reg_idx[rs]];
        int32_t reg_val = reg[reg_idx[rt]];
        int ed = addr & 3;
        int32_t tmp;
        if(ed == 0) 
            tmp = reg_val;

        if(ed == 1) 
            tmp = ((mem_val >> 24) << 24) + (reg_val & ((1<<24)-1));
    
        if(ed == 2) 
            tmp = ((mem_val >> 16) << 16) + (reg_val & ((1<<16)-1));
            
        if(ed == 3) 
            tmp = ((mem_val >> 8) << 8) + (reg_val & ((1<<8)-1));
        
        string word_str = bitset<word_size>(tmp).to_string();
        store(word_str, addr, "word");    
            
    }
    void MIPS_swr(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        uint32_t addr = get_reg(rs) + val;
        int32_t mem_val = reg[reg_idx[rs]];
        int32_t reg_val = reg[reg_idx[rt]];
        int ed = addr & 3;
        int32_t tmp;
        if(ed == 0) 
            tmp = ((reg_val & ((1<<8)-1)) << 24) + (mem_val & ((1<<24)-1));

        if(ed == 1) 
            tmp = ((reg_val & ((1<<16)-1)) << 16) + (mem_val & ((1<<16)-1));
    
        if(ed == 2) 
            tmp = ((reg_val & ((1<<24)-1)) << 8) + (mem_val & ((1<<8)-1));
            
        if(ed == 3) 
            tmp = reg_val;
        string word_str = bitset<word_size>(tmp).to_string();
        store(word_str, addr, "word");    
    }
    void MIPS_sb(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        uint32_t addr = val + get_reg(rs);
        string str = bitset<byte_size>((int8_t)get_reg(rt)).to_string();
        store(str, addr, "byte");    
    }
    void MIPS_sh(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        uint32_t addr = val + get_reg(rs);
        string str = bitset<half_size>((int16_t)get_reg(rt)).to_string();
        store(str, addr, "half");    
    }
    void MIPS_sw(const string &rt, const string &rs, const string &im){
        int32_t val = im2int(im);
        uint32_t addr = val + get_reg(rs);
        string str = bitset<word_size>(get_reg(rt)).to_string();
        store(str, addr, "word");    
    }
    void MIPS_j(const string &label){
        int32_t val = stoi(label, nullptr, 2);
        int32_t top = pc & (((1<<4)-1)<<28);
        pc = top + ((val<<2) & ((1<<28)-1));
    }
    void MIPS_jal(const string &label){
        int32_t val = stoi(label, nullptr, 2);
        reg[31] = pc;
        int32_t top = pc & (((1<<4)-1)<<28);
        pc = top + ((val<<2) & ((1<<28)-1));
        cout<<"jal"<<reg[31]<<" "<<pc<<endl;
    }

    void MIPS_syscall(){
        cout<<"syscall"<<reg[v0]<<endl;
        switch (reg[v0]){
            case 1: // print_int stored in a0
            {
                simout << reg[a0];
                simout.flush();
                break;
            }
            case 4: // print_string 
            {
                uint32_t addr = reg[a0];
                char ch = '\0';
                while (ch = load_byte_val(addr++)){
                    if(ch == '\0')break;
                    simout << ch;
                    simout.flush();
                }
                break;
            }
            case 5: // read_int and store in v0
            {
                simin >> reg[v0];
                break;
            }
            case 8: // read_string a0 buffer a1 string
            {
                // big problem here
                uint32_t addr = reg[a0];
                int32_t len = reg[a1];
                if (!len) break;
                for (int32_t i = 0; i < len - 1; i++){
                    char ch = getchar();
                    // If less than len-1, adds newline to end.
                    if (ch == '\0')
                        ch = '\n';
                    string s = bitset<byte_size>(int(ch)).to_string();
                    store(s, addr++, "byte");

                }
                // pads with null byte
                char ch = '\0';
                string s = bitset<byte_size>(int(ch)).to_string();
                store(s, addr++, "byte");
                break;
            }
            case 9: // sbrk return dyn_ed before increase, then increase dyn_ed
            {
                reg[v0] = get_addr(dyn_ed);
                dyn_ed += reg[a0];
                break;
            }
            case 10: // exit
            {
                throw_exception("syscall 10 exit");
                break;
            }
            case 11: // print_char
            {
                char ch = reg[a0] & ((1<<8)-1);
                simout << ch;
                simout.flush();
                break;
            }
            case 12: // read_char
            {
                reg[v0] = simin.get();
                break;
            }
            case 13: // open a0 file descriptor, a1 buffer, a2 length, return file descriptor 
            {
                // uint32_t addr = reg[a0];
                // string filename = "";
                // while(true){
                //     char ch = (char) load_byte_val(addr++);
                //     if (ch == '\0') break;
                //     filename += ch;
                // }
                // const char *file = filename.c_str();
                const char *filename = to_string(reg[a0]).c_str();

                reg[v0] = open(filename, reg[a1], reg[a2]);
                cout<<"fd is "<<reg[v0]<<endl;
                if(reg[v0] < 0)cout<<"error="<<errno<<endl;
                break;
            }
            case 14: // read arguments same as above, return read in a0
            {
                int len = reg[a2];
                uint8_t *buffer = new uint8_t[len];
                // read return # of bytes been read
                cout<<"reg[v0] is "<<reg[v0]<<endl;
                cout<<" file is"<<reg[a0]<<"and len is "<<len<<endl;
                cout<<(char) load_byte_val(reg[a0]) <<" "<< (char) load_byte_val(reg[a0] + 1) << endl;
                reg[v0] = read(reg[a0], buffer, len);
                if (reg[v0] == -1){
                    cout<<errno<<endl;
                    throw_exception("Fail to read");}
               


                uint32_t addr = reg[a1];
                for (int32_t i = 0; i < reg[v0]; i++){
                    string byte_str = bitset<byte_size>(*(buffer + i)).to_string();
                    store(byte_str, addr++, "byte");
                }
                delete[] buffer;
                break;
            }
            case 15: // write a0 file descriptor, a1 buffer, a2 len
            {
                // write to simout
                uint32_t addr = reg[a1];
                for (int32_t i = 0; i < reg[a2]; i++){
                    simout << load_byte_val(addr++);
                }
                
                break;
            }
            case 16:
            {
                close(reg[a0]);
                break;
            }
            case 17:
            {
                throw_exception("syscall 17 exit");
                break;
            }
            
        }
    }

};
int32_t &Simulator::get_reg(const string &reg_str){
        int idx = reg_idx[reg_str];
        return reg[idx];
}
int Simulator::get_idx(uint32_t addr){
    return (int)(addr - mem_base);
}
uint32_t Simulator::get_addr(int idx){
    return (uint32_t) (idx + mem_base);
}
void Simulator::store(string unit, uint32_t addr, string type){
    int idx = get_idx(addr);
    int inc, up_size;
    if(type == "word") inc = 4, up_size = word_size;
    if(type == "half") inc = 2, up_size = half_size;
    if(type == "byte") inc = 1, up_size = byte_size;
    int j = up_size;
    for(int i = idx; i < idx + inc; i++){
        for(int k = 0; k < byte_size; k++){
            memory[i][k] = unit[--j];
        }
            
    }

}
string Simulator::load(uint32_t addr, string type){
    int idx = get_idx(addr);
    int inc, up_size;
    if(type == "word") inc = 4, up_size = word_size;
    if(type == "half") inc = 2, up_size = half_size;
    if(type == "byte") inc = 1, up_size = byte_size;
    string unit(up_size, '0');
    int j = up_size;
    for(int i = idx; i < idx + inc; i++){
        for(int k = 0; k < byte_size; k++){
            // cout<<memory[i][k]<<endl;
            unit[--j] = memory[i][k];
        }
    }
    // cout<<*unit<<"haha"<<endl;
    return unit;

}
int32_t Simulator::load_word_val(uint32_t addr){
    string word_str = load(addr, "word");
    int32_t ret = stoul(word_str, nullptr, 2);
    return ret;
}
int8_t Simulator::load_byte_val(uint32_t addr){
    string byte_str = load(addr, "byte");
    int8_t ret = stoul(byte_str, nullptr, 2);
    return ret;
}
int16_t Simulator::load_half_val(uint32_t addr){
    string half_str = load(addr, "half");
    int16_t ret = stoul(half_str, nullptr, 2);
    return ret;
}

void Simulator::init(){
    for(int i = 0; i < memory_tot; i++)
        for(int j = 0; j < 8; j++) memory[i][j] = '0';


    reg[sp] = 0xa00000;
    reg[fp] = reg[sp];
    reg[gp] = 0x508000;

    for(int i = 0; i < 32; i++){
        string s = bitset<5>(i).to_string();
        reg_idx[s] = i;
    }


    // sotre static

    for(int i = 1; i < input.size(); i++){
        if(input[i].find(".text") != string::npos) break;
        char unit[word_size];
        for(int j = 0; j < word_size; j++)
            unit[j] = input[i][j];
        store(unit, get_addr(static_ed), "word");
        static_ed += 4;
    }
    dyn_ed = static_ed;

    // store text
    int st = 0;
    for(; st < input.size(); st++){
        string s = input[st];
        if(s.find(".text") != string::npos)
            break;
    }
    for(st = st + 1; st < input.size(); st++){
        string unit = "";
        for(int j = 0; j < word_size; j++)
            unit = unit + input[st][j];
        // cout<<"store "<<unit<<" at "<<text_ed<<endl;
        store(unit, get_addr(text_ed), "word");
        // cout<<load()<<" "<<memory[1]<<" "<<memory[2]<<" "<<memory[3]<<endl;
        text_ed += 4;
    }
}

void Simulator::simulate(){
    init();
    cout<<"simulate init done\n";
    // machine cycle
    pc = mem_base;
    int ins_count = 0;
    init_checkpoints(simcheck);
    while(pc >= mem_base && pc < get_addr(text_ed)){
        reg[32] = pc;
        checkpoint_memory(ins_count);
        checkpoint_register(ins_count);
        string word_str = load(pc, "word");
        pc += 4;
        excuate(word_str);
        ins_count += 1;
        if(ins_count<200)cout<<word_str<<" "<<pc<<" "<<reg[v0]<<" "<<reg[sp]<<endl;
    }
}


void Simulator::excuate(const string &instr){
    string rs, rt, rd, sa, funct, im, label;
    string opcode=instr.substr(0,6);
    // cout<<instr<<endl;
    if (opcode == "000000"){
        rs=instr.substr(6,5);
        rt=instr.substr(11,5);
        rd=instr.substr(16,5);
        sa=instr.substr(21,5);
        funct=instr.substr(26,6);
        
        if (funct=="000000" )
            MIPS_sll(rd,rt,sa);
        else if( funct=="000010" )
            MIPS_srl(rd,rt,sa);
        else if( funct=="000011" )
            MIPS_sra(rd,rt,sa);
        else if( funct=="000100" )
            MIPS_sllv(rd,rt,rs);
        else if( funct=="000110" )
            MIPS_srlv(rd,rt,rs);
        else if( funct=="000111" )
            MIPS_srav(rd,rt,rs);
        else if( funct=="001000" )
            MIPS_jr(rs);
        else if( funct=="001001" )
            MIPS_jalr(rd,rs);
        else if( funct=="001100" )
            MIPS_syscall();
        else if( funct=="010000" )
            MIPS_mfhi(rd);
        else if( funct=="010001" )
            MIPS_mthi(rs);
        else if( funct=="010010" )
            MIPS_mflo(rd);
        else if( funct=="010011" )
            MIPS_mtlo(rs);
        else if( funct=="011000" )
            MIPS_mult(rs,rt);
        else if( funct=="011001" )
            MIPS_multu(rs,rt);
        else if( funct=="011010" )
            MIPS_div(rs,rt);
        else if( funct=="011011" )
            MIPS_divu(rs,rt);
        else if( funct=="100000" )
            MIPS_add(rd,rs,rt);
        else if( funct=="100001" )
            MIPS_addu(rd,rs,rt);
        else if( funct=="100010" )
            MIPS_sub(rd,rs,rt);
        else if( funct=="100011" )
            MIPS_subu(rd,rs,rt);
        else if( funct=="100100" )
            MIPS_and(rd,rs,rt);
        else if( funct=="100101" )
            MIPS_or(rd,rs,rt);
        else if( funct=="100110" )
            MIPS_xor(rd,rs,rt);
        else if( funct=="100111" )
            MIPS_nor(rd,rs,rt);
        else if( funct=="101010" )
            MIPS_slt(rd,rs,rt);
        else if( funct=="101011" )
            MIPS_sltu(rd,rs,rt);
        else cout<<funct<<" R type not found\n";

    }
    else if( opcode == "000010" || opcode == "000011"){
        label=instr.substr(6);
        
        if (opcode=="000010" )
            MIPS_j(label);
        else if( opcode=="000011" )
            MIPS_jal(label);
        else cout<<label<<" J type not found\n";
    }  
    else if( opcode != "010000" && opcode != "010001" && opcode != "010010" && opcode != "010011"){
        rs=instr.substr(6,5);
        rt=instr.substr(11,5);
        im=instr.substr(16,16);
        if (opcode=="000001" and rt=="00000" )
            MIPS_bltz(rs,im);
        else if( opcode=="000001" and rt=="00001" )
            MIPS_bgez(rs,im);
        else if( opcode=="000100" )
            MIPS_beq(rs,rt,im);
        else if( opcode=="000101" )
            MIPS_bne(rs,rt,im);
        else if( opcode=="000110" )
            MIPS_blez(rs,im);
        else if( opcode=="000111" )
            MIPS_bgtz(rs,im);
        else if( opcode=="001000" )
            MIPS_addi(rt,rs,im);
        else if( opcode=="001001" )
            MIPS_addiu(rt,rs,im);
        else if( opcode=="001010" )
            MIPS_slti(rt,rs,im);
        else if( opcode=="001011" )
            MIPS_sltiu(rt,rs,im);
        else if( opcode=="001100" )
            MIPS_andi(rt,rs,im);
        else if( opcode=="001101" )
            MIPS_ori(rt,rs,im);
        else if( opcode=="001110" )
            MIPS_xori(rt,rs,im);
        else if( opcode=="001111" )
            MIPS_lui(rt,im);
        else if( opcode=="100000" )
            MIPS_lb(rt,rs,im);
        else if( opcode=="100001" )
            MIPS_lh(rt,rs,im);
        else if( opcode=="100011" )
            MIPS_lw(rt,rs,im);
        else if( opcode=="100100" )
            MIPS_lbu(rt,rs,im);
        else if( opcode=="100101" )
            MIPS_lhu(rt,rs,im);
        else if( opcode=="101000" )
            MIPS_sb(rt,rs,im);
        else if( opcode=="101001" )
            MIPS_sh(rt,rs,im);
        else if( opcode=="101011" )
            MIPS_sw(rt,rs,im);
        else if( opcode=="100010" )
            MIPS_lwl(rt,rs,im);
        else if( opcode=="100110" )
            MIPS_lwr(rt,rs,im);
        else if( opcode=="101010" )
            MIPS_swl(rt,rs,im);
        else if( opcode=="101110" )
            MIPS_swr(rt,rs,im);
        else cout<<"I type not found\n";
    }
        
}
void Simulator::init_checkpoints(char* checkpoint_file){
    FILE *fp = fopen(checkpoint_file, "r");
    int tmp, i = 0;
    while(fscanf(fp, "%d", &tmp) != EOF){
        checkpoints.insert(tmp);
    }
}
void Simulator::checkpoint_memory(int ins_count){
    if(!checkpoints.count(ins_count))return;
    string name = "My_memory_" + to_string(ins_count) + ".bin";
    FILE * fp = fopen(name.c_str(), "wb");
    for(int32_t i = 0x400000; i < (0x400000) + memory_tot; i += 4){
        string s = load(i, "word");
        // cout<<get_idx(i)<<endl;
        int32_t tmp = stoul(s, nullptr, 2);
        fwrite(&tmp, 4, 1, fp);
    }
    fclose(fp); 
}
void Simulator::checkpoint_register(int ins_count){
    if(!checkpoints.count(ins_count))return;
    string name = "My_register_" + to_string(ins_count) + ".bin";
    FILE * fp = fopen64(name.c_str(), "wb");
    for(int i = 0; i < 35; i++)
        fwrite(reg + i, 4, 1, fp);
    fclose(fp);
}
int main(int argc, char *argv[]){
    // cout<<argc<<endl;
    if (argc == 6){
        // assembler + simulator
        
        ifstream in1(argv[1]);
        ifstream in2(argv[2]);
        char * check_file = argv[3];
        ifstream in_file(argv[4]);
        ofstream out_file(argv[5]);
        if (!in1.is_open()){
            cout << argv[1] << " can not open" << endl;
            return 0;
        }
        if (!in2.is_open()){
            cout << argv[2] << " can not open" << endl;
            return 0;
        }

        if (!in_file.is_open()){
            cout << argv[4] << " can not open" << endl;
            return 0;
        }
        if (!out_file.is_open()){
            cout << argv[5] << " can not open" << endl;
            return 0;
        }
        try
        {
            Assembler assembler;
            assembler.assemble(in1, in2);
            cout<<"Assembler done\n";
            Simulator simulator(assembler.comb, check_file, in_file, out_file);
            simulator.simulate();
            cout<<"simulate done\n";
            in1.close();
            in2.close();
            in_file.close();
            out_file.close();
        }
        catch (const exception &e)
        {
            cerr << e.what() << endl;
        }
    }
    else
    {
        cout << "Wrong Argument Input" << endl;
    }
    return 0;


    
    
}
