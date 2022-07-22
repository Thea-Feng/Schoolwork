
#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<eigen3/Eigen/SVD>
#include<iostream>

using namespace std;
using namespace Eigen;

int main(){
    Matrix4d i, j;
    Vector4d v;
    v << 1, 1.5, 2, 3;
    i << 1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16;
    j << 4, 3, 2, 1,
        8, 7, 6, 5,
        12, 11, 10, 9,
        16, 15, 14, 13;
    cout<<"Matrix i is\n"<<i<<endl;
    cout<<"Matrix j is\n"<<j<<endl;
    cout<<"Matrix i + j is\n"<<i+j<<endl;
    cout<<"Matrix i * j is\n"<<i*j<<endl;
    cout<<"Matrix i * v is\n"<<(i*v).transpose()<<endl;
    return 0;
}