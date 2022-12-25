#include <queue>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

class Vertex
{
public:
    int depth = 0; // the depth on the tree
    int degree_cnt = 0; // number of degree of the node
    int weight = 0; // number of children nodes in the subtree

    int infected = 0; // 1: infected, 0: not infected yet

    std::vector<int> children; // children nodes
};

std::vector<Vertex> V;

// helper function
int degree_greedy(std::queue<int> &boundary)
{
    int head = boundary.front(), ans = head;
    
    do
    {
        int x = boundary.front();
        boundary.pop();
        boundary.push(x);
        
        if(V[x].degree_cnt > V[ans].degree_cnt)
        {
            ans = x;
        }

    } while (head != boundary.front());

    return ans;
}

// helper function
int weight_greedy(std::queue<int> &boundary)
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

// build tree
void build_tree(int now_idx)
{
    V[now_idx].weight = 1;
    V[now_idx].degree_cnt = 0;

    for(int child : V[now_idx].children)
    {
        V[child].depth = V[now_idx].depth + 1;

        build_tree(child);
        
        V[now_idx].degree_cnt++;
        V[now_idx].weight += V[child].weight;
    }
}

// simulation the vaccine status
void simulation_on_tree(int start_idx, int (*select_fn) (std::queue<int> &))
{
    std::queue<int> boundary;
    V[start_idx].infected = 1;

    for(int child : V[start_idx].children)
    {
        boundary.push(child);
    }

    for(int depth = 1; !boundary.empty(); depth++)
    {
        int choice_idx = select_fn(boundary);
        std::cout << "[round " << depth << "]: vaccinate " << choice_idx << std::endl; 
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
    // parse commond lines
    char * test_file = getCmdOption(argv, argv + argc, "-t");
    int use_weight_greedy = cmdOptionExists(argv, argv + argc, "-w");
    int use_degree_greedy = cmdOptionExists(argv, argv + argc, "-d");

    if(use_degree_greedy && use_weight_greedy)
    {
        std::cerr << "Please specify with one strategy!" << std::endl;
        return 1;
    }

    use_weight_greedy = 1 - use_degree_greedy;

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

    std::cerr << "Simulating using " 
        << (use_degree_greedy ? "degree-heavy" : "subtree-heavy")
        << " strategy" << std::endl;

    build_tree(1);

    simulation_on_tree(1, use_degree_greedy ? degree_greedy : weight_greedy);

    int saved_cnt = 0;
    
    for(const Vertex &v : V)
    {
        if(v.infected == 0)
        {
            saved_cnt++;
        }
    }

    std::cout << saved_cnt << " people are saved in total." << std::endl;

    return 0;
}