
#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<iostream>

using namespace std;
using namespace Eigen;
Vector4f trans(Vector4f a){
    return a / a(3);
}
// v[1, 1.5, 2, 3] and w[0, 1, 2, 4].
int main(int argc, char *argv[]){
    Vector4f v, w;
    v << 1, 1.5, 2, 3;
    w << 0, 1, 2, 4;
    v = trans(v);
    w = trans(w);
    Vector3f nv, nw;
    nv << v(0), v(1), v(2);
    nw << w(0), w(1), w(2);
    cout<<"Vector v is ["<<nv.transpose()<<"]\n";
    cout<<"Vector w is ["<<nw.transpose()<<"]\n";
    cout<<"The sum of v and w is ["<<(nv+nw).transpose()<<"]\n";
    cout<<"The inner product of v and w is "<<nv.dot(nw)<<endl;
    cout<<"The cross product of v and w is ["<<nv.cross(nw).transpose()<<"]\n";
    return 0;
}