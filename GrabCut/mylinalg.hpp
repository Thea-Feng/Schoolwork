#include<cmath>
#include<cstring>
#include<algorithm>
const double eps = 1e-10;
// #define mfor(i, b, c) for(int i = b; i < c; i++)

struct triple
{
    /* data */
    double mat[3];
    triple(){

    }
    triple(double x){
        mfor(i, 0, 3) mat[i] = x;
    }
    triple(double a, double b, double c){
        mat[0]=a;mat[1]=b;mat[2]=c;
    }
    double& operator [] (int idx){return mat[idx];}

    triple friend operator + (triple A, triple B){
        triple ans;
        mfor(i, 0, 3) ans[i] = A[i] + B[i];
        return ans;
    }
    triple friend operator - (triple A, triple B){
        triple ans;
        mfor(i, 0, 3) ans[i] = A[i] - B[i];
        return ans;
    }
    double friend operator * (triple A, triple B){
        double ans = 0;
        mfor(i, 0, 3) ans += A[i] * B[i];
        return ans;
    }
    triple friend operator * (triple A, double k){
        triple ans;
        mfor(i, 0, 3) ans[i] = A[i] * k;
        return ans;
    }
    triple friend operator / (triple A, int k){
        triple ans;
        mfor(i, 0, 3) ans[i] = A[i] / k;
        return ans;
    }
};
double square(triple A){return A * A;}

struct Matrix
{
    double v[3][3];
    Matrix(){}
    Matrix(bool t){
        memset(v, 0, sizeof(v));
        if(t)
            mfor(i, 0, 3)
                mfor(j, 0, 3)
                    v[i][j] = (i == j);
    }
    Matrix(triple x, bool form){
        memset(v, 0, sizeof(v));
        if(form)  mfor(i, 0, 3) v[0][i] = x[i];
        // x x x
        // 0 0 0
        // 0 0 0          
        else mfor(i, 0, 3) v[i][0] = x[i];
        // x 0 0
        // x 0 0
        // x 0 0
    }
    double* operator [] (int idx){return v[idx];}

    Matrix friend operator *(Matrix A, Matrix B){
        Matrix ans;
        ans = Matrix(0);
        mfor(i, 0, 3)
            mfor(j, 0, 3)
                mfor(k, 0, 3)
                    ans[i][j] += A[i][k] * B[k][j];
        return ans;
    }
    Matrix friend operator +(Matrix A, Matrix B){
        Matrix ans;
        mfor(i, 0, 3)
            mfor(j, 0, 3)
                ans[i][j] = A[i][j] + B[i][j];
        return ans;
    }
    bool friend operator ==(Matrix A, Matrix B){
        mfor(i, 0, 3)
            mfor(j, 0, 3)
                if(A[i][j] != B[i][j]) return 0;
        return 1;
    }
    Matrix friend operator *(Matrix A, double k){
        Matrix ans;
        mfor(i, 0, 3)
            mfor(j, 0, 3)
                ans[i][j] = A[i][j] * k;
        return ans;
    }
    Matrix friend operator /(Matrix A, double k){
        Matrix ans;
        mfor(i, 0, 3)
            mfor(j, 0, 3)
                ans[i][j] = A[i][j] / k;
        return ans;
    }
};
double det(Matrix M){
    return M[0][0]*(M[1][1]*M[2][2]-M[2][1]*M[1][2]) 
    + M[0][1]*(M[1][2]*M[2][0]-M[2][2]*M[1][0]) 
    + M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
}
Matrix inv(Matrix M){
    Matrix Inv;
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
        Inv[j][i]=M[(i+1)%3][(j+1)%3] * M[(i+2)%3][(j+2)%3] -M[(i+2)%3][(j+1)%3] * M[(i+1)%3][(j+2)%3];
    return Inv / det(M);

}

double Trace(Matrix M){return M[0][0]+M[1][1]+M[2][2];}
