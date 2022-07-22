// #pragma GCC optimize(3)
#include<queue>
#include<cstdio>
#include<iostream>
#define mfor(i, b, c) for(int i = (b); i < (c); i++)

#include "mylinalg.hpp"
#include "myguass.hpp"
#include "mykmeans.hpp"
#include "flow/graph.hpp"
#include "flow/Cut.hpp"

using namespace std;

const int MAXN = 1000000;
double beta, lamda=3; //gama???
int image_n, image_m, left_up_x, left_up_y, right_down_x, right_down_y;
int totcode;
triple pixel_z[MAXN];
int label_k[MAXN], label_alpha[MAXN], pixel_code[2101][2101];//!!!
GMM_model G[2];
int di[8]={0,-1,-1,-1,0,1,1,1};
int dj[8]={-1,-1,0,1,1,1,0,-1};

void Readimage();
void Initialization();
void Iterative_minimisation();
void Biggest();
void Writeimage();
void debug1(){
    freopen("debug1.out","w",stdout);
    mfor(i, 0, image_n)
        mfor(j, 0, image_m)
            printf("%d ",label_k[pixel_code[i][j]]);
}
int main(){
    Readimage();
    printf("read done\n");
    Initialization();
    printf("init done\n");
    Iterative_minimisation();
    printf("cutting done\n");
    Biggest();
    // printf("Biggest done\n");
    Writeimage();
    // printf("writing done\n");
    return 0;
}

void Writeimage(){
    freopen("data.out","w",stdout);
    int tt=0;
    mfor(i, 0, image_n)
        mfor(j, 0, image_m){
//        	tt++;
            mfor(t, 0, 3)
                printf("%d ", int(0.1+pixel_z[pixel_code[i][j]][t] * label_alpha[pixel_code[i][j]]));
            puts("");
        }
//    cout<<tt<<endl;/
}



void Readimage(){
    freopen("data.in","r",stdin);
    scanf("%d%d%d%d%d%d",&image_n,&image_m,&left_up_y,&left_up_x,&right_down_y,&right_down_x);
    printf("%d %d\n", image_n, image_m);
    mfor(i, 0, image_n)
        mfor(j, 0, image_m){
            int a,b,c;
            scanf("%d%d%d",&a,&b,&c);
            pixel_code[i][j] = ++totcode;
            pixel_z[totcode] = triple(a, b, c);
            if(i>left_up_y && i <right_down_y && j>left_up_x && j<right_down_x)
                label_alpha[totcode] = 1;
        }
}


void Build_GMM(bool fg = 0){
    G[0].init();
    G[1].init();
    mfor(i, 0, totcode)
        G[label_alpha[i]].component[label_k[i]].cal_1(pixel_z[i]);
    G[0].Process();
    G[1].Process();
    mfor(i, 0, totcode){
        G[label_alpha[i]].component[label_k[i]].cal_2(pixel_z[i]);    	
    }
    G[0].Done();
    G[1].Done();
    
}


void Build_graph(CutGraph& _graph){
    // freopen("mybuildg.out","w",stdout);
    int vcnt = image_n*image_m, ecnt = 2*(4*image_m*image_n - 3*(image_n+image_m) + 2);
    _graph = CutGraph(vcnt, ecnt);
    int bt=0;
    mfor(i, 0, image_n)
        mfor(j, 0, image_m){
            bt++;
            int now = pixel_code[i][j];
            triple pixz = pixel_z[now];
            int vnum = _graph.addVertex();
            double Sw = 0, Tw = 0;
            if(label_alpha[now]){
                Sw = G[0].getD(pixz);
                Tw = G[1].getD(pixz); 
            }
            else Sw = 10000; //break with T, inf with S(belong to S)
        //    if(Tw)
		// 		printf("%d %d %lf %lf\n", i, j, Sw, Tw);
            _graph.addVertexWeights(vnum, Sw, Tw);
            //V(α, z) = γ * dis(m,n)^(−1) [αn != αm] exp(−β(zm −zn))^2
            double val = 0;
            if(i){
                val = lamda * exp(-beta*square(pixz - pixel_z[pixel_code[i-1][j]]));
                _graph.addEdges(vnum, vnum - image_m, val);
            }
            if(j){
                val = lamda * exp(-beta*square(pixz - pixel_z[pixel_code[i][j-1]]));
                _graph.addEdges(vnum, vnum - 1, val);
            }
            if(i&&j){
                val = lamda/1.4142135 * exp(-beta*square(pixz - pixel_z[pixel_code[i-1][j-1]]));
                _graph.addEdges(vnum, vnum - image_m - 1, val);
            }
            if(i&&j<image_m-1){
                val = lamda/1.4142135 * exp(-beta*square(pixz - pixel_z[pixel_code[i-1][j+1]]));
                _graph.addEdges(vnum, vnum - image_m + 1, val);
            }

        }
}
void Minimization(CutGraph& _graph){
    Build_graph(_graph);
    printf("build graph done\n");
    _graph.maxFlow();
    printf("maxflow done\n");
}
bool change_alpha(CutGraph& _graph){
    bool ans=false;
    // freopen("debugchangealpha.out","w",stdout);
     mfor(i, left_up_y, right_down_y)
        mfor(j, left_up_x, right_down_x){
            // printf("%d %d %d %d\n",i, j, _graph.isSourceSegment(pixel_code[i][j]-1), label_alpha[pixel_code[i][j]]);
        }
    mfor(i, left_up_y, right_down_y)
        mfor(j, left_up_x, right_down_x){
            if(!_graph.isSourceSegment(pixel_code[i][j]-1) && label_alpha[pixel_code[i][j]]){
                label_alpha[pixel_code[i][j]] = 0;
                ans = true;
            }
        }
            
    return ans;
}

void Iterative_minimisation(){
    int cnt = 0;
    while(true){
        cnt++;
        printf("This is %d GMM\n", cnt);  
//        debug2();
		
        mfor(i, 0, totcode)
            if(label_alpha[i])
                label_k[i] = G[1].argminD(pixel_z[i]);  
   
         
        Build_GMM(1);        
        CutGraph graph;
        Minimization(graph);        
        printf("minimization done\n");
        if(!change_alpha(graph)){
//            cout<<"nochange"<<endl;
		return;
            break;
        }
        printf("this is %d times!!!!\n",cnt);
        if(cnt == 20)break;
    }
}


void Initialization(){
    // beta(alpha != ???)
    mfor(i, 0, image_n)
        mfor(j, 0, image_m)
            mfor(t, 0, 4){
                int newi = i + di[t], newj = j + dj[t];
                if(newi < 0 || newj < 0 || newi == image_n || newj == image_m)
                    continue;
                int newpos = pixel_code[newi][newj];
                beta += square(pixel_z[pixel_code[i][j]] - pixel_z[newpos]);
            }
    beta = 1.0/(2*beta/(4*image_n*image_m - 3*image_n - 3*image_m +2));
    KMEANS(0, label_k, label_alpha, pixel_z, totcode);
    KMEANS(1, label_k, label_alpha, pixel_z, totcode);
    Build_GMM();
        

}

void Biggest(void){
	int* bel = new int[image_m*image_n];
	int* count = new int[10000];
	int cnt=1;
	mfor(i, 0, image_n)
        mfor(j, 0, image_m){
			if(label_alpha[pixel_code[i][j]]){
				bel[pixel_code[i][j]]=0;
			}
		}
	
	mfor(i, 0, image_n)
        mfor(j, 0, image_m){
			if(label_alpha[pixel_code[i][j]]){
				if(!bel[pixel_code[i][j]]){
					bel[pixel_code[i][j]]=++cnt;
					count[cnt]=1;
					std::queue<int>rQ;
					std::queue<int>cQ;
					while(!rQ.empty())rQ.pop();
					while(!cQ.empty())cQ.pop();
					rQ.push(i);cQ.push(j);
					while(!rQ.empty()){
						int nowi = rQ.front();rQ.pop();
						int nowj = cQ.front();cQ.pop();
						for(int d=0;d<8;++d){
							int nxti = nowi + di[d];
							int nxtj = nowj + dj[d];
							int nxtc = pixel_code[nxti][nxtj];
							if(label_alpha[nxtc]&&(!bel[nxtc])){
								bel[nxtc] = cnt;
								count[cnt]++;
								rQ.push(nxti);
								cQ.push(nxtj);
							}
						}
					}
				}
			}
		}
	
	int maxt=1;
	for(int i=2;i<=cnt;++i)
		if(count[maxt]<count[i])
			maxt=i;
	mfor(i, 0, image_n)
        mfor(j, 0, image_m){
			if(label_alpha[pixel_code[i][j]]&&bel[pixel_code[i][j]]!=maxt){
				bool fg=0;
				for(int d=0;d<8;++d){
						int nxti = i + di[d];
						int nxtj = j + dj[d];
						int nxtc = pixel_code[nxti][nxtj];
						if(bel[nxtc]==maxt){fg=1;}
					}
				if(!fg)
				label_alpha[pixel_code[i][j]]=0;
			}
		}
	
	delete [] count;
	delete [] bel;
	return ;
}

