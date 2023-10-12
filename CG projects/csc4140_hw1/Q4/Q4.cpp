#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<eigen3/Eigen/SVD>
#include<iostream>

using namespace std;
using namespace Eigen;
const double PI = acos(-1.0);

int main(){
    Vector4d v, w, mid;
    v << 1, 
        2, 
        3,
        1;
    w << 4, 
        5, 
        6,
        1;
    Matrix4d P1, P2;
    P1 << 1, 0, 0, -w(0),
        0, 1, 0, -w(1),
        0, 0, 1, -w(2),
        0, 0, 0, 1;
    P2 << 1, 0, 0, w(0),
        0, 1, 0, w(1),
        0, 0, 1, w(2),
        0, 0, 0, 1;
    double thetax = 45 * PI / 180, thetay = 30 * PI / 180, thetaz = 60 * PI /180;
    Matrix4d z, y, x;
    double c = cos(thetax), s = sin(thetax);
    x << 1, 0, 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1;
    c = cos(thetay); s = sin(thetay);
    y << c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1;
    c = cos(thetaz); s = sin(thetaz);
    z << c, -s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    Vector4d ret;
    ret = P2 * x * y * z * P1 * v;
    cout<<"The point after rotation is ("<<ret(0)<<" "<<ret(1)<<" "<<ret(2)<<")\n";
    return 0;
}