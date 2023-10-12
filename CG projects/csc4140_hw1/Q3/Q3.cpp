#include<cstdio>
#include<iostream>
#include<cstring>
#include<opencv2/opencv.hpp>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<eigen3/Eigen/SVD>
#include<sstream>

using namespace std;
using namespace Eigen;
using namespace cv;

MatrixXd img2Mat(const Mat &img){
    MatrixXd M = MatrixXd(img.rows, img.cols);
    for(int i = 0; i < img.rows; i++)
        for(int j = 0; j < img.cols; j++) 
            M(i, j) = 1.0/255*img.at<uchar>(i, j);
    return M;
}
Mat mat2Img(MatrixXd M){
    Mat img(M.rows(), M.cols(), CV_8UC1);
    for(int i = 0; i < M.rows(); i++)
        for(int j = 0; j < M.cols(); j++){
            int x = M(i, j) * 255;
            if(x > 255) x = 255;
            if(x < 0) x = 0;
            img.at<uchar>(i, j) = x;
        }
    return img;
}

void imgShow(const char *name, const Mat &img){
    namedWindow(name, WINDOW_AUTOSIZE );
    imshow(name, img);
}

void Savefeature(const MatrixXd& S, const MatrixXd& U, const MatrixXd& V, int dim){
    MatrixXd NS = S.block(0, 0, S.rows(), dim);
    // cout <<NS;
    MatrixXd NV = V.block(0, 0, V.rows(), dim);
    MatrixXd M = U * NS * NV.transpose();
    Mat img = mat2Img(M);
    char name[20];
    if(dim == 1)strcpy(name, "Feature map with 1");
    else if(dim == 10)strcpy(name, "Feature map with 10");
    else strcpy(name, "Feature map with 50");

    imgShow(name, img);
    strcat(name,".jpg");
    imwrite(name, img);

    // cout<<img;
}


int main(int argc, char *argv[]){
    if(argc != 2){
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat image;
    image = imread( argv[1], 0 );
    if(!image.data){
        printf("No image data\n");
        return -1;
    }

    imgShow("Init", image);
    MatrixXd A;
    A = img2Mat(image);
    BDCSVD<MatrixXd> svd(A, ComputeThinU | ComputeThinV);
    MatrixXd V = svd.matrixV(), U = svd.matrixU();
    MatrixXd S = U.inverse() * A * V.transpose().inverse();
    Savefeature(S, U, V, 1);
    Savefeature(S, U, V, 10);
    Savefeature(S, U, V, 50);   
    // U(:,:)*S(:,1)*V(:,1)’
    // cout<<"Done"<<endl;
    waitKey(0);
    return 0;
}