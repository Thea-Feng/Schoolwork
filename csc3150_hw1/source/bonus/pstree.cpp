

#include <bits/stdc++.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stack>
#include <vector>
using namespace std;
const int MAXN = 1e5 + 5;
struct Proc {
	int pid, ppid, uid, gid;
	int idx;
	int cnt;
	string name;
	vector<int> children;
};
Proc Tr[MAXN];
int tot = 0, indx[MAXN];
map<int, string> UID;
map<string, int> mp;
void print_idx(int idx)
{
	Proc proc = Tr[idx];
	printf("Pid is %d, PPid is %d and name is ", proc.pid, proc.ppid);
	std::cout << proc.name << "\n";
}
vector<std::string> readtxt_name(string file)
{
	ifstream infile;
	infile.open(file.data());
	vector<std::string> txt_name;
	string str;

	while (getline(infile, str)) {
		txt_name.push_back(str);
	}
	infile.close();
	return txt_name;
}
vector<string> stk;
void print_tree(int root, int mode, int uid = 0)
{
	// stack <string> stk;
	// queue <int> q;
	string s(Tr[root].name.length() + 1 + (root != 1) +
			 (mode == 1) * (to_string(Tr[root].pid).length() + 2) +
			 (mode == 3 && (Tr[root].uid != uid)) *
				 (UID[Tr[root].uid].length() + 2),
		 ' ');
	if (mode == 0 || mode == 3) {
		if (Tr[root].cnt == 1)
			cout << Tr[root].name;
		else if (Tr[root].cnt > 1) {
			cout << Tr[root].cnt << "*[" << Tr[root].name;
			if (Tr[root].children.size() == 0)
				cout << "]";
		}
		if (mode == 3)
			if (uid != Tr[root].uid) {
				cout << "(" << UID[Tr[root].uid] << ")";
			}
	}
	if (mode == 1)
		cout << Tr[root].name << "(" << Tr[root].pid << ")";
	if (mode == 2)
		cout << Tr[root].name;
	// <<" "<<Tr[root].pid<<" "<<Tr[root].ppid;
	for (int i = 0; i < Tr[root].children.size(); i++) {
		if (i == 0) {
			stk.push_back(s + "|");
			cout << "---";
			int nxt = Tr[root].children[i];
			print_tree(nxt, mode, Tr[root].uid);
		} else {
			int nxt = Tr[root].children[i];
			if (mode == 1 || mode == 2) {
				for (int j = 0; j < stk.size(); j++)
					cout << stk[j];
				cout << "-";
				print_tree(nxt, mode, Tr[root].uid);
			} else if (mode == 0 || mode == 3)
				if (Tr[nxt].cnt) {
					for (int j = 0; j < stk.size(); j++)
						cout << stk[j];
					cout << "-";
					print_tree(nxt, mode, Tr[root].uid);
					if (Tr[root].cnt > 1)
						cout << "]";
				}
		}
	}

	if (Tr[root].children.size() == 0)
		cout << "\n";
	else
		stk.pop_back();
}

void process(int root, bool fg)
{
	for (int i = 0; i < Tr[root].children.size(); i++) {
		int nxt = Tr[root].children[i];
		if (mp.find(Tr[nxt].name) != mp.end() &&
		    Tr[nxt].children.size() == 0) {
			int c = mp[Tr[nxt].name];
			Tr[c].cnt++;
			Tr[nxt].cnt--;
		} else
			mp[Tr[nxt].name] = Tr[nxt].idx;
	}
	mp.erase(mp.begin(), mp.end());
	if (!fg)
		for (int i = 0; i < Tr[root].children.size(); i++)
			for (int j = i + 1; j < Tr[root].children.size(); j++) {
				Proc x = Tr[Tr[root].children[i]],
				     y = Tr[Tr[root].children[j]];
				if (x.name > y.name ||
				    (x.name == y.name &&
				     x.children.size() < y.children.size()))
					swap(Tr[root].children[j],
					     Tr[root].children[i]);
			}
	for (int i = 0; i < Tr[root].children.size(); i++) {
		int nxt = Tr[root].children[i];
		if (Tr[nxt].cnt)
			process(nxt, fg);
	}
	string lst = "";
	int ccnt;
	for (int i = 0; i < Tr[root].children.size(); i++) {
		int nxt = Tr[root].children[i];
		if (lst != Tr[nxt].name) {
			lst = Tr[nxt].name;
			ccnt = nxt;
		} else {
			if (Tr[nxt].children.size() ==
				    Tr[ccnt].children.size() &&
			    Tr[nxt].name == "node") {
				Tr[nxt].cnt--;
				Tr[ccnt].cnt++;
			} else
				ccnt = nxt;
		}
	}
}
void read_user()
{
	FILE *fp;
	fp = freopen("/etc/passwd", "r", stdin);
	string str;
	while (getline(cin, str)) {
		int pos = str.find(":");
		string user = str.substr(0, pos);
		pos = str.find(":", pos + 1);
		int nxtpos = str.find(":", pos + 1);
		int id = stoi(str.substr(pos + 1, nxtpos - pos - 1));
		UID[id] = user;
	}
	fclose(fp);
}
int main(int argc, char *argv[])
{
	vector<std::string> txt_name;
	vector<std::string> final_path_txt_name;

	string path = "data.txt";
	std::string str_temp;
	char ss[20];

	txt_name = readtxt_name(path);
	read_user();

	int i = 0;
	for (vector<string>::iterator it = txt_name.begin();
	     it != txt_name.end(); ++it) {
		str_temp = *it;
		strcpy(ss, str_temp.c_str());
		string path_final = "/proc/";

		path_final = path_final + str_temp;
		ifstream tmp;
		tmp.open(path_final.data());
		if (tmp.is_open() == 0)
			continue;

		string str;
		Proc proc;
		bool fg1 = 0, fg2 = 0, fg3 = 0;
		while (getline(tmp, str)) {
			// int pos;
			if (str.find("Name:") != -1)
				proc.name = str.substr(6, str.length() - 2);
			if (!fg1 && str.find("Pid:") != -1) {
				fg1 = 1;
				proc.pid =
					stoi(str.substr(5, str.length() - 2));
			}
			if (!fg2 && str.find("PPid:") != -1) {
				fg2 = 1;
				proc.ppid =
					stoi(str.substr(6, str.length() - 2));
				if (proc.pid != 1 && !indx[proc.ppid]) {
					fg3 = 1;
					break;
				}
			}
			if (str.find("NStgid:") != -1) {
				int ctt = stoi(str.substr(8, str.length() - 2));
				if (ctt != proc.pid) {
					proc.ppid = ctt;
					proc.name = "{" + proc.name + "}";
				}
			}
			if (str.find("Uid:") != -1) {
				int ctt = stoi(str.substr(5, 8));
				proc.uid = ctt;
				// if(Tr[indx[proc.ppid]].uid) proc.uid = 0;
			}
			if (str.find("NSpgid:") != -1) {
				int ctt = stoi(str.substr(8, str.length() - 2));
				proc.gid = ctt;
				// if(Tr[indx[proc.ppid]].uid) proc.uid = 0;
			}
		}

		if (fg3)
			continue;
		proc.idx = ++tot;
		proc.cnt = 1;
		int c = proc.pid;
		indx[c] = tot;
		Tr[tot] = proc;
		Tr[indx[proc.ppid]].children.push_back(tot);
		final_path_txt_name.push_back(path_final);
	}
	// print_idx(1);

	string cc = "";
	if (argc > 1) {
		for (int i = 1; i < argc; i++)
			cc = cc + argv[i];
	}
	if (cc == "-n")
		process(indx[1], 1);
	else
		process(indx[1], 0);
	if (cc == "-g")
		for (int i = 1; i <= tot; i++)
			Tr[i].pid = Tr[i].gid;
	if (argc == 1) {
		print_tree(indx[1], 0);
	} else {
		if (cc == "-p")
			print_tree(indx[1], 1);
		if (cc == "-c")
			print_tree(indx[1], 2);
		if (cc == "-n")
			print_tree(indx[1], 0);
		if (cc == "-u")
			print_tree(indx[1], 3);
		if (cc == "-g")
			print_tree(indx[1], 1);
	}
	return 0;
}