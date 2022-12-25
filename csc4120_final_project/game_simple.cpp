#include <queue>
#include <vector>
#include <chrono>
#include <random>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#define RED -1
#define BLUE 1


class Vertex
{
public:
    int color = 0; // -1 or 1, red or blue
    int color_diff = 0;
    int depth = 0; // the depth on the tree
    int degree_cnt = 0; // number of degree of the node
    int weight = 0; // number of children nodes in the subtree

    int infected = 0; // 1: infected, 0: not infected yet

    std::vector<int> children; // children nodes
};

std::vector<Vertex> V;

class Player
{
public:
    int color;
    int save_cnt;
    virtual int select_vertex(std::queue<int> &boundary) = 0;

    Player() { }
    Player(int col) : color(col), save_cnt(0) { }
};

class SelfishPlayer
    : public Player
{
public:

    SelfishPlayer() { }
    SelfishPlayer(int col) : Player(col) { }

    int select_vertex(std::queue<int> &boundary) override
    {
        int head = boundary.front(), ans = head;
    
        do
        {
            int x = boundary.front();
            boundary.pop();
            boundary.push(x);
            
            if(V[x].color_diff * color > V[ans].color_diff * color)
            {
                ans = x;
            }

        } while (head != boundary.front());

        return ans;
    }
};

class SubtreeHeavyPlayer
    : public Player
{
public:

    SubtreeHeavyPlayer() { }
    SubtreeHeavyPlayer(int col): Player(col) { }

    int select_vertex(std::queue<int> &boundary) override
    {
        int head = boundary.front(), ans = head;
    
        do
        {
            int x = boundary.front();
            boundary.pop();
            boundary.push(x);
            
            if(V[x].weight > V[ans].weight)
            {
                ans = x;
            }

        } while (head != boundary.front());

        return ans;
    }
};


Player * player[2] = { nullptr, nullptr }; // 0 red, 1 blue


// build tree
void build_tree(int now_idx)
{
    V[now_idx].weight = 1;
    V[now_idx].degree_cnt = 0;
    V[now_idx].color_diff = V[now_idx].color;

    for(int child : V[now_idx].children)
    {
        V[child].depth = V[now_idx].depth + 1;

        build_tree(child);
        
        V[now_idx].degree_cnt++;
        V[now_idx].weight += V[child].weight;
        V[now_idx].color_diff += V[child].color_diff;
    }
}

// simulation the vaccine status
void simulation_on_tree(int start_idx)
{
    std::queue<int> boundary;
    V[start_idx].infected = 1;

    for(int child : V[start_idx].children)
    {
        boundary.push(child);
    }

    for(int depth = 1, now = std::rand() % 2; !boundary.empty(); depth++, now = 1 - now)
    {
        int choice_idx = player[now]->select_vertex(boundary);
        while(!boundary.empty())
        {
            int x = boundary.front();
            if(V[x].depth != depth)
            {
                break;
            }

            boundary.pop();

            if(x == choice_idx)
            {
                continue;
            }

            V[x].infected = 1;

            for(int child : V[x].children)
            {
                boundary.push(child);
            }
        }
    }
}


void init_vertices(int num_vertices)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::srand(seed);

    int * colors = new int[num_vertices + 1];
    int off = std::rand() % 2;

    for(int i = 1; i <= num_vertices; i++)
    {
        colors[i] = (i + off) % 2 ? RED : BLUE;
    }

    std::shuffle(colors + 1, colors + num_vertices + 1, std::default_random_engine(seed));

    for(int i = 1; i <= num_vertices; i++)
    {
        V[i].color = colors[i];
    }

    delete[] colors;
}

int game_strategy(char * arg)
{
    if(arg == nullptr)
    {
        return -1;
    }

    int ret_val = -1;

    if(std::strcmp(arg, "SSSS") == 0)
    {
        ret_val = 0;
    }
    else if(std::strcmp(arg, "SSSH") == 0)
    {
        ret_val = 1;
    }
    else if(std::strcmp(arg, "SHSS") == 0)
    {
        ret_val = 2;
    }
    else if(std::strcmp(arg, "SHSH") == 0)
    {
        ret_val = 3;
    }

    if(ret_val == -1)
    {
        return -1;
    }

    if(ret_val & 2)
    {
        player[0] = new SubtreeHeavyPlayer(RED);
    }
    else
    {
        player[0] = new SelfishPlayer(RED);
    }

    if(ret_val & 1)
    {
        player[1] = new SubtreeHeavyPlayer(BLUE);
    }
    else
    {
        player[1] = new SelfishPlayer(BLUE);
    }

    return ret_val;
}

char * getCmdOption(char **first, char **last, const std::string & opt)
{
    char ** iter = std::find(first, last, opt);
    
    if(iter != last && iter + 1 != last)
    {
        return *(iter + 1);
    }

    return nullptr;
}

bool cmdOptionExists(char **first, char **last, const std::string & opt)
{
    return std::find(first, last, opt) != last;
}

int main(int argc, char **argv)
{
    std::ios::sync_with_stdio(false);

    // parse commond lines
    char * test_file = getCmdOption(argv, argv + argc, "-t");
    char * strategy = getCmdOption(argv, argv + argc, "-s");
    int stat = game_strategy(strategy);

    if(stat == -1)
    {
        std::cerr << "Please specify a valid strategy!" << std::endl;
        return 1;
    }

    if(test_file == nullptr)
    {
        std::cerr << "Please specify the path of the test file!" << std::endl;
        return 1;
    }

    // open the test file
    std::ifstream file_in(test_file);
    std::string buffer;
    std::istringstream line;

    if(file_in.fail())
    {
        std::cerr << "Error in opening the test case file!" << std::endl;
        return 1;
    }

    std::cerr << "Reading test file:";
    int num_vertices;

    std::getline(file_in, buffer);
    line.str(buffer);
    line >> num_vertices;

    V.resize(num_vertices + 1);
    V[0].infected = 1;

    for(int i = 1; i <= num_vertices && std::getline(file_in, buffer); i++)
    {
        int idx, x;
        line.clear();
        line.str(buffer);
        line >> idx;

        while(line >> x)
        {
            V[idx].children.push_back(x);
        }
    }

    file_in.close();

    std::cerr << " done." << std::endl;

    std::cerr << "Red Player: " << ((stat & 2) ? "SH" : "SS") << std::endl;
    std::cerr << "Blue Player: " << ((stat & 1) ? "SH" : "SS") << std::endl;

    init_vertices(num_vertices);

    build_tree(1);

    simulation_on_tree(1);

    int saved_cnt = 0;
    
    for(const Vertex &v : V)
    {
        if(v.infected == 0)
        {
            saved_cnt++;
            if(v.color == RED)
            {
                player[0]->save_cnt++;
            }
            else if(v.color == BLUE)
            {
                player[1]->save_cnt++;
            }
        }
    }

    std::cout << saved_cnt << " people are saved in total." << std::endl;
    std::cout << "Saved Reds: " << player[0]->save_cnt << std::endl;
    std::cout << "Saved Blues: " << player[1]->save_cnt << std::endl;
    std::cout << ((player[0]->save_cnt > player[1]->save_cnt) ? "Red" : "Blue") << " wins" << std::endl;

    delete player[0];
    delete player[1];

    return 0;
}