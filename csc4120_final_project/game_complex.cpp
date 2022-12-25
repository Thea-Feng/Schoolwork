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

class SegtreeNode
{
public:
    // data stored
    int max_color_diff;
    int min_color_diff;
    int max_weight;

    int max_color_index;
    int min_color_index;
    int max_weight_index;

    // auxiliary
    int saved;
    int save_tag;
    int color_tag;
    int weight_tag;

    void to_default()
    {
        max_color_diff = -0x3f3f3f3f;
        min_color_diff = 0x3f3f3f3f;
        max_weight = 0;
        saved = 0;
        color_tag = 0;
        weight_tag = 0;
    }

    void set_saved()
    {
        to_default();
        saved = 1;
        save_tag = 1;
    }

    void apply_color_tag(int tag)
    {
        color_tag += tag;
        max_color_diff += tag;
        min_color_diff += tag;
    }

    void apply_weight_tag(int tag)
    {
        weight_tag += tag;
        max_weight += tag;
    }
};

class Segtree
{
public:
    int length;
    std::vector<SegtreeNode> tree;

    Segtree(): length(0) { }
    Segtree(int seg_length)
        : length(seg_length)
    {
        tree.resize(length * 4 + 1);
        init(1, length, 1);
    }

    void init(int l, int r, int now)
    {
        tree[now].to_default();
        if(l == r)
        {
            return ;
        }

        int mid = (l + r) >> 1;
        init(l, mid, now << 1);
        init(mid + 1, r, now << 1 | 1);
    }

    void insert(int idx, int v_index, int color_sum, int weight)
    {
        insert(1, this->length, idx, 1, v_index, color_sum, weight);
    }

    void save_range(int first_index, int last_index)
    {
        save_range(1, length, first_index, last_index, 1);
    }

    void update(int first, int last, int col_diff, int weight_diff)
    {
        update_range(1, length, first, last, 1, col_diff, weight_diff);
    }

    int max_weight_index(int first, int last, int &ans, int &max_val)
    {
        ans = -1, max_val = -0x3f3f3f3f;
        
        query_max_weight(1, length, first, last, 1, max_val, ans);

        return ans;
    }

    int max_color_diff(int first, int last, int &ans, int &max_val)
    {
        ans = -1, max_val = -0x3f3f3f3f;

        query_max_color_diff(1, length, first, last, 1, max_val, ans);

        return ans;
    }

    int min_color_diff(int first, int last, int &ans, int &min_val)
    {
        ans = -1, min_val = 0x3f3f3f3f;

        query_min_color_diff(1, length, first, last, 1, min_val, ans);

        return ans;
    }

    int is_saved(int index)
    {
        return query_saved(1, length, index, 1);
    }

    const SegtreeNode * snapshot(int index)
    {
        return query_everything(1, length, index, 1);
    }

private:
    void pushup(int now)
    {
        tree[now].max_color_diff = std::max(tree[now << 1].max_color_diff, tree[now << 1 | 1].max_color_diff);
        tree[now].max_color_index = (tree[now << 1].max_color_diff > tree[now << 1 | 1].max_color_diff) ? 
            tree[now << 1].max_color_index : tree[now << 1 | 1].max_color_index; 
        
        tree[now].min_color_diff = std::min(tree[now << 1].min_color_diff, tree[now << 1 | 1].min_color_diff);
        tree[now].min_color_index = (tree[now << 1].min_color_diff < tree[now << 1 | 1].min_color_diff) ? 
            tree[now << 1].min_color_index : tree[now << 1 | 1].min_color_index;

        tree[now].max_weight = std::max(tree[now << 1].max_weight, tree[now << 1 | 1].max_weight);
        tree[now].max_weight_index = (tree[now << 1].max_weight > tree[now << 1 | 1].max_weight) ?
            tree[now << 1].max_weight_index : tree[now << 1 | 1].max_weight_index;
    }

    void pushdown(int now)
    {
        if(tree[now].save_tag)
        {
            tree[now << 1].set_saved();
            tree[now << 1 | 1].set_saved();
            tree[now].save_tag = 0;
        }

        if(tree[now].color_tag)
        {
            tree[now << 1].apply_color_tag(tree[now].color_tag);
            tree[now << 1 | 1].apply_color_tag(tree[now].color_tag);
            tree[now].color_tag = 0;
        }

        if(tree[now].weight_tag)
        {
            tree[now << 1].apply_weight_tag(tree[now].weight_tag);
            tree[now << 1 | 1].apply_weight_tag(tree[now].weight_tag);
            tree[now].weight_tag = 0;
        }
    }

    void insert(int l, int r, int i, int now, int v_index, int color, int weight)
    {
        if(l == r)
        {
            tree[now].max_color_diff = tree[now].min_color_diff = color;
            tree[now].max_weight = weight;
            tree[now].max_color_index = tree[now].min_color_index = tree[now].max_weight_index = v_index;
            return ;
        }
        
        pushdown(now);

        int mid = (l + r) >> 1;

        if(i <= mid)
        {
            insert(l, mid, i, now << 1, v_index, color, weight);
        }
        else
        {
            insert(mid + 1, r, i,now << 1 | 1, v_index, color, weight);
        }

        pushup(now);
    }

    void save_range(int l, int r, int saved_l, int saved_r, int now)
    {
        if(saved_l > r || l > saved_r)
        {
            return ;
        }

        if(saved_l <= l && r <= saved_r)
        {
            tree[now].set_saved();
            return ;
        }

        pushdown(now);

        int mid = (l + r) >> 1;

        save_range(l, mid, saved_l, saved_r, now << 1);
        save_range(mid + 1, r, saved_l, saved_r, now << 1 | 1);

        pushup(now);
    }

    void update_range(int l, int r, int upd_l, int upd_r, int now, int upd_color, int upd_weight)
    {
        if(upd_l > r || l > upd_r)
        {
            return ;
        }

        if(upd_l <= l && r <= upd_r)
        {
            tree[now].apply_color_tag(upd_color);
            tree[now].apply_weight_tag(upd_weight);
            return ;
        }

        pushdown(now);

        int mid = (l + r) >> 1;

        update_range(l, mid, upd_l, upd_r, now << 1, upd_color, upd_weight);
        update_range(mid + 1, r, upd_l, upd_r, now << 1 | 1, upd_color, upd_weight);

        pushup(now);
    }

    void query_max_weight(int l, int r, int query_l, int query_r, int now, int & max_weight, int & index)
    {
        if(query_l > r || l > query_r)
        {
            return ;
        }

        if(query_l <= l && r <= query_r)
        {
            if(max_weight < tree[now].max_weight)
            {
                max_weight = tree[now].max_weight;
                index = tree[now].max_weight_index;
            }

            return ;
        }

        pushdown(now);

        int mid = (l + r) >> 1;

        query_max_weight(l, mid, query_l, query_r, now << 1, max_weight, index);
        query_max_weight(mid + 1, r, query_l, query_r, now << 1 | 1, max_weight, index);
    }

    void query_max_color_diff(int l, int r, int query_l, int query_r, int now, int & max_color_diff, int & index)
    {
        if(query_l > r || l > query_r)
        {
            return ;
        }

        if(query_l <= l && r <= query_r)
        {
            if(max_color_diff < tree[now].max_color_diff)
            {
                max_color_diff = tree[now].max_color_diff;
                index = tree[now].max_color_index;
            }

            return ;
        }

        pushdown(now);

        int mid = (l + r) >> 1;

        query_max_color_diff(l, mid, query_l, query_r, now << 1, max_color_diff, index);
        query_max_color_diff(mid + 1, r, query_l, query_r, now << 1 | 1, max_color_diff, index);
    }

    void query_min_color_diff(int l, int r, int query_l, int query_r, int now, int & min_color_diff, int & index)
    {
        if(query_l > r || l > query_r)
        {
            return ;
        }

        if(query_l <= l && r <= query_r)
        {
            if(min_color_diff > tree[now].min_color_diff)
            {
                min_color_diff = tree[now].min_color_diff;
                index = tree[now].min_color_index;
            }

            return ;
        }

        pushdown(now);

        int mid = (l + r) >> 1;

        query_min_color_diff(l, mid, query_l, query_r, now << 1, min_color_diff, index);
        query_min_color_diff(mid + 1, r, query_l, query_r, now << 1 | 1, min_color_diff, index);
    }

    int query_saved(int l, int r, int i, int now)
    {
        if(l == r)
        {
            return tree[now].saved;
        }

        pushdown(now);

        int mid = (l + r) >> 1;
        
        if(i <= mid)
        {
            return query_saved(l, mid, i, now << 1);
        }

        return query_saved(mid + 1, r, i, now << 1 | 1);
    }

    SegtreeNode * query_everything(int l, int r, int i, int now)
    {
        if(l == r)
        {
            return &tree[now];
        }

        pushdown(now);

        int mid = (l + r) >> 1;
        
        if(i <= mid)
        {
            return query_everything(l, mid, i, now << 1);
        }

        return query_everything(mid + 1, r, i, now << 1 | 1);
    }
};

Segtree * segtree = nullptr;

class Vertex
{
public:
    int color = 0; // -1 or 1, red or blue
    int color_diff = 0;
    int depth = 0; // the depth on the tree
    int degree_cnt = 0; // number of degree of the node
    int weight = 0; // number of children nodes in the subtree

    int parent = 0;

    int infected = 0; // 1: infected, 0: not infected yet

    int max_child = 0;
    int chain_top;
    int pre, post;

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

    void update_situation(int x)
    {
        const SegtreeNode * it = segtree->snapshot(V[x].pre);
        int col_diff = it->max_color_diff;
        int weight = it->max_weight;

        segtree->save_range(V[x].pre, V[x].post);

        int now = x;
        while(now != 0)
        {
            segtree->update(V[V[now].chain_top].pre, V[now].pre, -col_diff, -weight);    
            now = V[V[now].chain_top].parent;
        }
    }
};

class SelfishPlayer
    : public Player
{
public:

    SelfishPlayer() { }
    SelfishPlayer(int col) : Player(col) { }

    int select_vertex(std::queue<int> &boundary) override
    {
        int head = boundary.front(), ans = head, weight_ans = (-color) * 0x3f3f3f3f;

        do
        {
            int x = boundary.front();
            boundary.pop();
            boundary.push(x);

            int ans_x, weight_x;

            if(color == RED) 
            {
                segtree->min_color_diff(V[x].pre, V[x].post, ans_x, weight_x);
                if(weight_x < weight_ans)
                {
                    weight_ans = weight_x;
                    ans = ans_x;
                }
            }
            else
            {
                segtree->max_color_diff(V[x].pre, V[x].post, ans_x, weight_x);
                if(weight_x > weight_ans)
                {
                    weight_ans = weight_x;
                    ans = ans_x;
                }
            }
            

        } while (head != boundary.front());

        update_situation(ans);

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
        int head = boundary.front(), ans = head, weight_ans = -0x3f3f3f3f;

        do
        {
            int x = boundary.front();
            boundary.pop();
            boundary.push(x);

            int ans_x, weight_x;

            segtree->max_weight_index(V[x].pre, V[x].post, ans_x, weight_x);

            if(weight_x > weight_ans)
            {
                weight_ans = weight_x;
                ans = ans_x;
            }
            

        } while (head != boundary.front());

        update_situation(ans);

        std::cout << ans << " saves " << weight_ans << std::endl;;
        return ans;
    }
};


Player * player[2] = { nullptr, nullptr }; // 0 red, 1 blue

int time_stamp = 0;

// build tree
void build_tree(int now_idx)
{
    V[now_idx].weight = 1;
    V[now_idx].degree_cnt = 0;
    V[now_idx].color_diff = V[now_idx].color;

    int max_weight = -1;

    for(int child : V[now_idx].children)
    {
        V[child].parent = now_idx;
        V[child].depth = V[now_idx].depth + 1;

        build_tree(child);

        if(V[child].weight > max_weight)
        {
            max_weight = V[child].weight;
            V[now_idx].max_child = child;
        }
        
        V[now_idx].degree_cnt++;
        V[now_idx].weight += V[child].weight;
        V[now_idx].color_diff += V[child].color_diff;
    }
}

void build_segtree(int now_idx, int top_idx)
{
    if(now_idx == 0)
    {
        return ;
    }
    
    V[now_idx].pre = ++time_stamp;
    V[now_idx].chain_top = top_idx;

    segtree->insert(V[now_idx].pre, now_idx, V[now_idx].color_diff, V[now_idx].weight);

    build_segtree(V[now_idx].max_child, top_idx);

    for(int child : V[now_idx].children)
    {
        if(child == V[now_idx].max_child)
        {
            continue;
        }

        build_segtree(child, child);
    }


    V[now_idx].post = time_stamp;
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
        std::cout << "[round " << depth << "] " << (now ? "RED" : "BLUE") << " saves " ;
        int choice_idx = player[now]->select_vertex(boundary);
        
        while(!boundary.empty())
        {
            int x = boundary.front();
            if(V[x].depth != depth)
            {
                break;
            }

            boundary.pop();

            if(segtree->is_saved(V[x].pre))
            {
                continue;
            }

            V[x].infected = 1;

            for(int child : V[x].children)
            {
                if(! (segtree->is_saved(V[child].pre)) )
                {
                    boundary.push(child);
                }
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
    segtree = new Segtree(num_vertices);

    build_tree(1);
    build_segtree(1, 1);


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
    delete segtree;

    return 0;
}