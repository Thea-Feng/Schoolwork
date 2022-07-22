#include<stdlib.h>
#include<ctime>
using namespace std;
double getrandnumber(int lower, int upper){
    return lower + (upper - lower) * (rand()%10001) / (10000.0);
}
triple miu[5], low, up;
int num[5];


void KMEANS(int opt, int *gpk, int *alpha, triple *pixel_z, int tot){
    srand(time(NULL));
    low[0] = low[1] = low[2] = inf;
    mfor(i, 0, tot)
        if(opt == alpha[i]){
            mfor(t, 0, 3)
                low[t] = min(low[t], pixel_z[i][t]), up[t] = max(up[t], pixel_z[i][t]);
        }            
    mfor(k, 0, 5)
        mfor(t, 0, 3)
            miu[k][t] = getrandnumber(low[t], up[t]);

    double newJ = inf, oldJ = inf;
    int tt=0;
    do{
        tt++;
        oldJ = newJ; newJ = 0;
        mfor(k, 0, 5)num[k] = 0;
        // asign k
        mfor(i, 0, tot)
            if(opt == alpha[i]){
                int ret = 0;
                mfor(k, 1, 5){
                    if(square(pixel_z[i] - miu[k]) < square(pixel_z[i] - miu[ret]))
                        ret = k; 
                }
                gpk[i] = ret;
                num[ret]++;
            }
        mfor(k, 0, 5) miu[k] = triple(0);
        mfor(i, 0, tot) miu[gpk[i]] = miu[gpk[i]] + pixel_z[i];
        // for empty cluster
        mfor(k, 0, 5)
            if(!num[k]){
                int ret = 0;
                mfor(t, 1, 5)
                    if(num[t] > num[ret])
                        ret = t;
                int now = 0; double maxdis = -inf;
                mfor(i, 0, tot)
                    if(opt == alpha[i] && gpk[i] == ret){
                        double dis = square(pixel_z[i] - (miu[ret]/num[ret]));
                        if(dis > maxdis) maxdis = dis, now = i;
                    }
                num[ret]--; num[k]++;
                miu[ret] = miu[ret] - pixel_z[now]; miu[k] = miu[k] + pixel_z[now];
                gpk[now] = k;
            }
        //renew miu
        mfor(k, 0, 5) miu[k] = miu[k] / num[k];
        mfor(i, 0, tot)
            newJ += square(pixel_z[i] - miu[gpk[i]]);
          printf("iterate %d\n",tt);
    }while(oldJ - newJ > 1e-3);
    
}