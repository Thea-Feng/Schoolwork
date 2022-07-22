// #define mfor(i, b, c) for(int i = b; i < c; i++)
const double PI = acos(-1.0);
const double inf = 1e18; ////???????????????????????

struct Guassion
{
    /* data */
    triple miu;
    Matrix sigma;
    int Mass;
    
    double pdf(triple x){
    	Matrix A = Matrix(x-miu, 1);
		Matrix B = inv(sigma); 
		Matrix C = Matrix(x-miu,0);
		Matrix D = A * B;
		Matrix tmp = D * C;
		 
//        Matrix tmp = Matrix(x-miu, 1)*inv(sigma)*Matrix(x-miu, 0);
        double ans = 1.0/(pow(2*PI, 1.5)*sqrt(det(sigma)))*exp(tmp[0][0]*(-0.5));
        return ans;
    }

    void init(){
        miu = triple(0);
        sigma = Matrix(0);
        Mass = 0;
    }
    void cal_1(triple x){
        miu = miu + x;
        Mass += 1;
    }
    void cal_2(triple x){
        sigma = sigma + Matrix(x - miu, 0) * Matrix(x - miu, 1); // 巧用矩阵乘法
    }
    void ep(){
        mfor(i, 0, 3)
            mfor(j, 0, 3)
                if(sigma[i][j] < 1e-7)
                    sigma[i][j] = 1e-7;
    }

};

struct GMM_model
{
    Guassion component[5];
    double weight[5];
    int totMass;
    void init(){//checked
        mfor(i, 0, 5){
            component[i].init();
            weight[i] = 0;
        }
        totMass = 0;
    }
    void Process(){//checked
        mfor(i, 0, 5)totMass += component[i].Mass;
        mfor(i, 0, 5){
            weight[i] = component[i].Mass * 1.0 / totMass;
            if(component[i].Mass > 0)
            component[i].miu = component[i].miu / component[i].Mass;
        }
    }
    void Done(){//checked
        mfor(i, 0, 5)
			component[i].ep();
        mfor(i, 0, 5) 
            if(component[i].Mass > 1)
                component[i].sigma = component[i].sigma / (1.0*component[i].Mass - 1);
    }
    double getD(triple pt){
        int k = argminD(pt);
		double ans=-log(weight[k]) + 0.5*log(det(component[k].sigma))
			   +0.5*(Matrix(pt - component[k].miu,1)*inv(component[k].sigma)*Matrix(pt - component[k].miu,0))[0][0];
        return ans;
    }
    
    int argminD(triple pt){//checked
         int ret = 0;
         double ans = 0;
         mfor(t, 0, 5){
             double newpro = weight[t] * component[t].pdf(pt);
             if(newpro > ans) ans = newpro, ret = t;
         }
        return ret;
    }
};
