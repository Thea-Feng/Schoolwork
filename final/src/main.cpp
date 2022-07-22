#include<bits/stdc++.h>
#pragma GCC optimize(3, "Ofast", "inline")
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<eigen3/Eigen/SVD>
#include <filesystem>
#include <fstream>
#include <iostream>
const int active = 1, inner = 0, boundary = -1;
#define PI std::acos(-1)
struct  Vertex;
bool debug = false;
struct Edge
{
    Vertex *a, *b, *op;
    Eigen::Vector3d center;
    Edge *prev, *next;
    int status = active;
};
// const int MAXN = 1e5, MAXM=2e5, MAX_T=1e2;
int MAX_X, MAX_Y, MAX_Z;
float ball_radius;

struct Vertex{
    int idx=0;
    Eigen::Vector3d pos, normal;
    bool used = false;
    std::vector<Edge*> edges;
};

std::vector< std::vector<Vertex> > Voxels;

struct Triangle
{
    Vertex *vi, *vj, *vk;

    Eigen::Vector3d normal(){
        return ((vi->pos - vj->pos).cross(vi->pos - vk->pos)).normalized();
    }

};
struct Front
{
    std::vector<Edge*> frontedge;
    bool inFront(Vertex *p){
        for(auto& e: p->edges){
            if(e->status == active) return true;
        }
        return false;
    }
    void deleteEdge(Edge* edge) {
		edge->status = inner;
	}
    void glue(Edge* a, Edge* b) {

        if (a->next == b && a->prev == b && b->next == a && b->prev == a) {
            deleteEdge(a);
            deleteEdge(b);
            return;
        }
        if (a->next == b && b->prev == a) {
            a->prev->next = b->next;
            b->next->prev = a->prev;
            deleteEdge(a);
            deleteEdge(b);
            return;
        }
        if (a->prev == b && b->next == a) {
            a->next->prev = b->prev;
            b->prev->next = a->next;
            deleteEdge(a);
            deleteEdge(b);
            return;
        }
        a->prev->next = b->next;
        b->next->prev = a->prev;
        a->next->prev = b->prev;
        b->prev->next = a->next;
        deleteEdge(a);
        deleteEdge(b);
    }

    auto findReverseEdgeOnFront(Edge* edge) -> Edge* {
        for (auto& e : edge->a->edges)
            if (e->a == edge->b)
                return e;
        return nullptr;
    }
    auto join(Edge* e_ij, Vertex *vk, Eigen::Vector3d ballCenter, std::deque<Edge>& edges){
        auto& e_ik = edges.emplace_back(Edge{e_ij->a, vk, e_ij->b, ballCenter});
        auto& e_kj = edges.emplace_back(Edge{vk, e_ij->b, e_ij->a, ballCenter});

        e_ik.next = &e_kj;
        e_ik.prev = e_ij->prev;
        e_ij->prev->next = &e_ik;
        e_ij->a->edges.push_back(&e_ik);

        e_kj.prev = &e_ik;
        e_kj.next = e_ij->next;
        e_ij->next->prev = &e_kj;
        e_ij->b->edges.push_back(&e_kj);

        vk->used = true;
        vk->edges.push_back(&e_ik);
        vk->edges.push_back(&e_kj);

        frontedge.push_back(&e_ik);
        frontedge.push_back(&e_kj);
        deleteEdge(e_ij);

        return std::vector<Edge*> {&e_ik, &e_kj};
    }

    auto getActiveEdge() -> Edge* {
        while(!frontedge.empty()) {
            auto* e = frontedge.back();
            if(e->status == active) return e;
            frontedge.pop_back();
        }
        return nullptr;
    }

    
};

Eigen::Vector3d low, up;
inline Eigen::Vector3d voxelIndex(Eigen::Vector3d now){
    Eigen::Vector3d index = (now - low) / (2*ball_radius);
    index[0] = std::min(int(index[0]), MAX_X-1);
    index[1] = std::min(int(index[1]), MAX_Y-1);
    index[2] = std::min(int(index[2]), MAX_Z-1);
    return index;
}

void init(const std::vector<Vertex>& cloud){
    low = cloud[0].pos, up = cloud[0].pos;
    std::cout<<"init begin\n";
    for (auto& p : cloud) {
        for (auto i = 0; i < 3; i++) {
            low[i] = std::min(low[i], p.pos[i]);
            up[i] = std::max(up[i], p.pos[i]);
        }
    }


    MAX_X = (int)std::max(1.0, ceil((up[0] - low[0]) / (2*ball_radius)));
    MAX_Y = (int)std::max(1.0, ceil((up[1] - low[1]) / (2*ball_radius)));
    MAX_Z = (int)std::max(1.0, ceil((up[2] - low[2]) / (2*ball_radius)));
    // if(debug){
        std::cout<<"dimension of X Y Z are "<<MAX_X<<" "<<MAX_Y<<" "<<MAX_Z<<std::endl;
    //     std::cout<<"low is "<<low<<" up is "<<up<<std::endl;
    // }

    Voxels.resize(MAX_X * MAX_Y * MAX_Z);
    int tot_point = 0;
    for (const auto& p : cloud){
        Eigen::Vector3d index = voxelIndex(p.pos);
        int idx = index[2]*MAX_X*MAX_Y + index[1]*MAX_X + index[0];
        Voxels[idx].push_back(Vertex{tot_point++, p.pos, p.normal});
    }

}

inline float get_distance(Eigen::Vector3d A, Eigen::Vector3d B){
    return (A - B).norm(); 
}


auto getNeighbors(Eigen::Vector3d now, std::initializer_list<Eigen::Vector3d> ignore){
    std::vector<Vertex*> indices;
    Eigen::Vector3d index = voxelIndex(now);
    int dx[3]={-1,0,1},dy[3]={-1,0,1},dz[3]={-1,0,1};

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            for(int k = 0; k < 3; k++){
                int x=index[0]+dx[i], y=index[1]+dy[j], z=index[2]+dz[k];
                if(x>=0 && y>=0 && z>=0 && x<MAX_X && y<MAX_Y && z<MAX_Z){
                    int idx = z*MAX_X*MAX_Y + y*MAX_X + x;
                    // if(debug)std::cout<<"idx "<<idx<<" "<<Voxels[idx].size()<<"\n";
                    for(int t = 0; t < Voxels[idx].size(); t++){
                       // get center ball voxels to check empty
                        float d = get_distance(Voxels[idx][t].pos, now);
                        if(d <= 2*ball_radius && std::find(begin(ignore), end(ignore), Voxels[idx][t].pos) == end(ignore)) {
                            indices.push_back(&Voxels[idx][t]);
                        }

                    }
                }
            }
    return indices;
}

bool checkBallEmpty(Eigen::Vector3d center, const std::vector<Vertex*>& points){
    return !std::any_of(begin(points), end(points), [&](Vertex* p) {
            float d = get_distance(p->pos, center);
            return d*d < ball_radius * ball_radius - 1e-4f; // TODO epsilon
        });
}
bool computeBallCenter(Triangle f, Eigen::Vector3d &ballCenter) {
    Eigen::Vector3d ac = f.vk->pos - f.vi->pos;
    Eigen::Vector3d ab = f.vj->pos - f.vi->pos;
    Eigen::Vector3d abXac = ab.cross(ac);
    Eigen::Vector3d toCircumCircleCenter = (abXac.cross(ab) * ac.dot(ac) + ac.cross(abXac) * ab.dot(ab)) / (2 * abXac.dot(abXac));
    Eigen::Vector3d circumCircleCenter = f.vi->pos + toCircumCircleCenter;

    const auto heightSquared = ball_radius * ball_radius - toCircumCircleCenter.dot(toCircumCircleCenter);
    if (heightSquared < 0)
        return false;
    ballCenter = circumCircleCenter + f.normal() * std::sqrt(heightSquared);
    return true;
}
bool isOriented(Eigen::Vector3d A, Eigen::Vector3d B){
    return A.dot(B) >= 0;
}
bool findSeed(Triangle &res, Eigen::Vector3d &ballCenter){
    //transform

    //enumerate
    if(debug)std::cout<<"FindSeed begin\n";
    for(auto& voxel: Voxels)
    for(auto& vi: voxel){
        if(vi.used) continue;
        auto indices = getNeighbors(vi.pos, {vi.pos});
        std::sort(begin(indices), end(indices), [&](Vertex* a, Vertex* b) {
            return get_distance(a->pos, vi.pos) < get_distance(b->pos, vi.pos);
        });
        // if(debug) std::cout<<"Size of neigbhour "<<indices.size()<<std::endl;
        for(auto& vj: indices){
            if(vj->used) continue; //isUsed?
            for(auto& vk: indices){
                if(vk == vj || vk->used) continue;
                Triangle f{&vi, vj, vk};
                if(!isOriented(f.normal(), vi.normal) || !isOriented(f.normal(), vj->normal) || !isOriented(f.normal(), vk->normal)) continue;
                if(!computeBallCenter(f, ballCenter))continue;
                bool empty = checkBallEmpty(ballCenter, indices);
                // if(debug)std::cout<<vi.pos<<"\n"<<vj->pos<<"\n"<<vk->pos<<"\n"<<ballCenter<<" "<<empty<<" "<<indices.size()<<"\n";

                if(empty) {
                    // if(debug) std::cout<<vi.pos<<"\n"<<vj->pos<<"\n"<<vk->pos<<"\n"<<ballCenter<<" "<<indices.size()<<" empty "<<empty<<"\n";

                    vi.used = true;
                    vj->used = true;
                    vk->used = true;
                    res = f;
                    return true;
                }
                // abort();
            }
        }
    }
    return false;
    
}
struct  sigmak
{
    Vertex *point;
    Eigen::Vector3d center;
};

auto ball_pivot(const Edge* e, bool check = false){
    if(check) std::cout<<"ball_pivot begin\n";
    auto mid = (e->a->pos + e->b->pos) / 2.0f;
    Eigen::Vector3d oldCenterVec = (e->center - mid).normalized();
    auto neighborhood = getNeighbors(mid, {e->a->pos, e->b->pos, e->op->pos});

    int counter = 0;
    counter++;

    auto minAngle = std::numeric_limits<float>::max();
    Vertex* pointWithMinAngle = nullptr;
    Eigen::Vector3d centerOfMin{};
    // std::stringstream ss;
    auto i = 0;
    int minNumber = 0;
    if(check)std::cout<<neighborhood.size()<<"is size of neigh\n";
    
    for (const auto& p : neighborhood) {
        i++;
        Eigen::Vector3d newFaceNormal = Triangle{e->b, e->a, p}.normal();

        // this check is not in the paper: all points' normals must point into the same half-space
        if (newFaceNormal.dot(p->normal) < 0)
            continue;
        Eigen::Vector3d c;
        if(!computeBallCenter(Triangle{e->b, e->a, p}, c))
            continue;
        // this check is not in the paper: the ball center must always be above the triangle
        Eigen::Vector3d newCenterVec = (c - mid).normalized();
        float newCenterFaceDot = newCenterVec.dot(newFaceNormal);
        if (newCenterFaceDot < 0) {
            if (check) std::cout << i << ".    " << p->pos << " ball center " << c << " underneath triangle\n";
            continue;
        }

        // this check is not in the paper: points to which we already have an inner edge are not considered
        bool innerEdge = false;
        for (const auto* ee : p->edges) {
            const auto* otherPoint = ee->a == p ? ee->b : ee->a;
            if (ee->status == inner && (otherPoint == e->a || otherPoint == e->b)) {
                innerEdge = true;
                if (check) std::cout << i << ".    " << p->pos << " inner edge exists\n";
            }

        }

        if(innerEdge) continue;
        auto angle = std::acos(std::min(std::max(oldCenterVec.dot(newCenterVec), -1.0), 1.0));
        if ((newCenterVec.cross(oldCenterVec)).dot(e->a->pos - e->b->pos) < 0)
            angle += PI; // >180`
        if (angle <= minAngle - 0.0000005) {
            minAngle = angle;
            pointWithMinAngle = p;
            centerOfMin = c;
            minNumber = i;
        }
        if (check) std::cout << i << ".    " <<p->idx<<" "<<pointWithMinAngle->idx<<"  "<<"  " << p->pos << " center " << c << " angle " << angle << " newCenterFaceDot " << newCenterFaceDot << "\n";
    }

    if (minAngle != std::numeric_limits<float>::max()) {
        if (check) std::cout <<pointWithMinAngle->idx<<"  "  << pointWithMinAngle->pos << " center " << centerOfMin << " angle " << minAngle << "\n";
        if(checkBallEmpty(centerOfMin, neighborhood)) 

        return sigmak{pointWithMinAngle, centerOfMin};
    }
    // if(debug) std::cout<<"Not found pivot point\n";
    Eigen::Vector3d nul; nul << 0, 0, 0;
    return sigmak{nullptr, nul};
    
}


std::vector<Triangle> Tmesh;

auto reconstruct(const std::vector<Vertex>& cloud){
    init(cloud);
    if(debug) std::cout<<"Init done\n";

    std::vector<std::array<Eigen::Vector3d, 3> > mesh;
    while (true){
        int mesh_count = 0;
        Triangle seed;
        Eigen::Vector3d ballCenter;
        std::deque<Edge> edges;
        // std::cout << "Searching a seed\n";
        if (findSeed(seed, ballCenter)){
            if(debug) std::cout<<"Find seed done\n";
            mesh_count++;
            mesh.push_back({seed.vi->pos, seed.vj->pos, seed.vk->pos});
            Tmesh.push_back(seed);
            auto& e0 = edges.emplace_back(Edge{seed.vi, seed.vj, seed.vk, ballCenter});
            auto& e1 = edges.emplace_back(Edge{seed.vj, seed.vk, seed.vi, ballCenter});
            auto& e2 = edges.emplace_back(Edge{seed.vk, seed.vi, seed.vj, ballCenter});
            e0.prev = e1.next = &e2;
            e0.next = e2.prev = &e1;
            e1.prev = e2.next = &e0;
            seed.vi->edges = { &e0, &e2 }; 
            seed.vj->edges = { &e0, &e1 };
            seed.vk->edges = { &e1, &e2 };
            Front front;
            front.frontedge.push_back(&e0);
            front.frontedge.push_back(&e1);
            front.frontedge.push_back(&e2);
            int cnt = 0, cct = 0;
            // freopen("vertex.txt","w",stdout);
            while (auto e_ij = front.getActiveEdge()){
                //std::cout << "Testing edge " << *edge << "\n";
                if(e_ij == nullptr) break;
                bool checksign = false;
                auto vk = ball_pivot(e_ij, checksign);
                if(checksign)
                    std::cout<<"haha"<<e_ij->a->idx<<" "<<e_ij->b->idx<<" "<<(vk.point != nullptr)<<"\n";
                if(checksign && (vk.point != nullptr))    std::cout<<(!(vk.point->used))<<" "<<front.inFront(vk.point)<<"\n";
                if (vk.point != nullptr && (!(vk.point->used) || front.inFront(vk.point)))
                {
                    cnt++;
                    // if(cnt % 2000 == 0 &&debug) std::cout<<cnt/2000<<" "<<cct<<" "<<"e_ij is"<<e_ij->a->pos<<"\n"<<e_ij->b->pos<<"\n"<<vk.point->pos<<"\n";
				    // if(checksign)
                    // std::cout<<cnt<<" "<<cct<<"  "<<e_ij->a->idx<<" "<<vk.point->idx<<" "<<e_ij->b->idx<<"\n";
                    Triangle f{e_ij->a, vk.point, e_ij->b};
                    Tmesh.push_back(f);
                    mesh.push_back({e_ij->a->pos, vk.point->pos, e_ij->b->pos});
                    auto vec = front.join(e_ij, vk.point, vk.center, edges);
                    auto e_ik = vec[0], e_kj = vec[1];
                    if (auto* e_ki = front.findReverseEdgeOnFront(e_ik)) {cct++;front.glue(e_ik, e_ki);}
				    if (auto* e_jk = front.findReverseEdgeOnFront(e_kj)) {cct++;front.glue(e_kj, e_jk);}
                    // abort();
                }
                else{
				    e_ij->status = boundary;
                }
            }
            // fclose(stdout);
            // abort();
            if(debug) std::cout<<"mesh_count is "<<mesh_count<<"\n";
        }
		else {std::cout << "No seed triangle found\n"; break;}


    }
    return mesh;
}

inline auto loadXYZ(std::filesystem::path path){
	std::ifstream f{path};
	if (!f)
		throw std::runtime_error("Faild to read file " + path.string());

	std::vector<Vertex> cloud;
    // a xyz file each line consists of 6 numbers
	while (!f.eof()) {
		Vertex p;
		f >> p.pos[0] >> p.pos[1] >> p.pos[2] >> p.normal[0] >> p.normal[1] >> p.normal[2];
		cloud.push_back(p);
	}
	return cloud;
}

inline void outputTriangles(char * filename, std::vector<std::array<Eigen::Vector3d, 3> >& triangles, std::vector<Triangle>& Tmesh, const std::vector<Vertex>& points) {
	
    freopen("../result/trimesh.txt","w",stdout);
    std::string s(filename);
    printf("%s\n",s.c_str());
    printf("[[%6f, %6f, %6f]", points[0].pos[0], points[0].pos[1], points[0].pos[2]);
    for(int i = 1; i < points.size(); i++){
        printf(", [%6f, %6f, %6f]", points[i].pos[0], points[i].pos[1], points[i].pos[2]);
    }
    printf("]\n");

    printf("[[%d, %d, %d]", Tmesh[0].vi->idx, Tmesh[0].vj->idx, Tmesh[0].vk->idx);
    for(int i = 1; i < Tmesh.size(); i++){
        printf(", [%d, %d, %d]", Tmesh[i].vi->idx, Tmesh[i].vj->idx, Tmesh[i].vk->idx);
    }
    printf("]\n");
    // if(debug)
    // for(auto& t: triangles){
    //     // cc++;
    //     // printf("%8d\n %.4lf %.4lf %.4lf ",cc, t[0][0], t[0][1], t[0][2]);
    //     // printf("%.4lf %.4lf %.4lf ", t[1][0], t[1][1], t[1][2]);
    //     // printf("%.4lf %.4lf %.4lf\n", t[2][0], t[2][1], t[2][2]);
	// 	std::cout<<cc<<"\n"<<std::fixed<<std::setprecision(3)<<t[0][0]<<" "<<t[0][1]<<" "<<t[0][2]<<" "<<t[1][0]<<" "<<t[1][1]<<" "<<t[1][2]<<" "<<t[2][0]<<" "<<t[2][1]<<" "<<t[2][2]<<"\n";

    // }
    fclose(stdout);
}
auto Start(const std::vector<Vertex>& points){
    const auto start = std::chrono::high_resolution_clock::now();
    auto result = reconstruct(points);
    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count();
    std::cerr << " Point: " << points.size() << " Triangles: " << result.size() <<" Total time: "<< time << " Triangle/s: " << result.size() / time << '\n';
    return result;
}
int main(int argc, char *argv[]){
    // convert ply to xyz, generate a xyz file
    if(argc != 3) {std::cout<<"Wrong arguments number\n"; return 0;}
    std::ofstream ofs;
    ofs.open("tmp.txt",std::ios::out);
    ofs<<argv[1];
    ofs.close();
    system("python3 loadfile.py");
    system("rm tmp.txt");
    const auto cloud = loadXYZ("../result/input.xyz");
    if(debug) std::cout<<"Load done\n";
    ball_radius = 1.0f * atof(argv[2]);
	auto mesh = Start(cloud);
    if(debug) std::cout<<"Reconstruct done\n";
	// outputTriangles("bunny_mesh.stl", mesh, Tmesh, cloud);
	outputTriangles(argv[1], mesh, Tmesh, cloud);

    // show reconstructed mesh and export a dae file
    system("python3 exportfile.py");
}