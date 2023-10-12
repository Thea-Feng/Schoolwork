#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
// add some other header files you need

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;
    // std::clog << "view" << std::endl << view << std::endl;  // check data

    return view;
}


Eigen::Matrix4f get_model_matrix(float rotation_angle, Eigen::Vector3f T, Eigen::Vector3f S, Eigen::Vector3f P0, Eigen::Vector3f P1)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    //Step 1: Build the Translation Matrix M_trans:
    Eigen::Matrix4f M_trans;
    M_trans << 1, 0, 0, T(0),   0, 1, 0, T(1),  0, 0, 1, T(2),   0, 0, 0, 1;
    //Step 2: Build the Scale Matrix S_trans:
    Eigen::Matrix4f S_trans;
    S_trans << S(0), 0, 0, 0,   0, S(1), 0, 0,  0, 0, S(2), 0,   0, 0, 0, 1;
    //Step 3: Implement Rodrigues' Rotation Formular, rotation by angle theta around axix u, then get the model matrix
	// The axis u is determined by two points, u = P1-P0: Eigen::Vector3f P0 ,Eigen::Vector3f P1  
    // Create the model matrix for rotating the triangle around a given axis. // Hint: normalize axis first
    Eigen::Vector3f axis = P1 - P0;
    axis.normalize();
    double radian = rotation_angle*MY_PI/180;
    float c = cos(radian), s = sin(radian);
    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f R, N;
    // Eigen::Matrix3f N;
    N << 0, -axis(2), axis(1),   axis(2), 0, -axis(0),   -axis(1), axis(0), 0;
    R = c*I + (1 - c)*axis*axis.transpose() + s*N;
    Eigen::Matrix4f R_mat;
    R_mat << R(0,0), R(0,1), R(0,2), 0,   R(1,0), R(1, 1), R(1, 2), 0,   R(2, 0), R(2, 1), R(2, 2), 0,   0, 0, 0, 1;
	//Step 4: Use Eigen's "AngleAxisf" to verify your Rotation
	Eigen::AngleAxisf rotation_vector(radian, Vector3f(axis[0], axis[1], axis[2]));  
	Eigen::Matrix3f rotation_matrix;
	rotation_matrix = rotation_vector.toRotationMatrix();
    // if (R  == rotation_matrix) std::cout<<"Right\n";
    // else std::cout<<"wrong\n";
    model =  S_trans * R_mat * M_trans * model;
    // std::clog << "view" << std::endl << model << std::endl;  // check data

	return model;
}



Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Implement this function


    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // frustum -> cubic
    Eigen::Matrix4f M_per2orth;
    M_per2orth << zNear, 0, 0, 0,   0, zNear, 0, 0,   0, 0, zNear+zFar, -zNear*zFar,   0, 0, 1, 0;
    // orthographic projection
    float t = zNear * tan(eye_fov*MY_PI/180/2);
    float r = aspect_ratio * t;
    float l = -r, b = -t;
    // squash all transformations
    Eigen::Matrix4f M_orths, M_ortht;
    M_orths << 2/(r-l), 0, 0, 0,   0, 2/(t-b), 0, 0,   0, 0, 2/(zNear-zFar), 0,   0, 0, 0, 1;
    M_ortht << 1, 0, 0, -(r+l)/2,   0, 1, 0, -(t+b)/2,   0, 0, 1, -(zNear+zFar)/2,   0, 0, 0, 1;

    projection = M_orths * M_ortht * M_per2orth * projection;

    // std::clog << "projection" << std::endl << projection << std::endl; //check
    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "result.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(1024, 1024);

    // *******************************In this area where you can change parameters**********************************************************
    ///// Paramter One
    // // define your eye position "eye_pos" to a proper position
    Eigen::Vector3f eye_pos = {0, 0, 5};
    Eigen::Vector3f line_color = {65, 155, 205};
    // //define a triangle named by "pos" and "ind"
    // //added parameters for get_projection_matrix(float eye_fov, float aspect_ratio,float zNear, float zFar)
    std::vector<Eigen::Vector3f> pos{{4, 0, -2}, {0, 2, -2}, {-2, 0, -2}};
    std::vector<Eigen::Vector3i> ind{{0,1,2}};
    float eye_fov = 45, aspect_ratio = 1, zNear = -1, zFar = -40;
    Eigen::Vector3f T(0, 0, 0), S(0.5, 0.5, 0.5), P0(0, 1, 8), P1(0, 1, 4);
    Eigen::Vector3f axis(0, 0, 1);
    


    ////  Here is Parameter Two for another triangle
    // Eigen::Vector3f eye_pos = {0, 0, 5};
    // Eigen::Vector3f line_color = {255, 153, 18};
    // std::vector<Eigen::Vector3f> pos{{0, 1, -2}, {-1, -1, 2}, {1, -1, 2}};
    // std::vector<Eigen::Vector3i> ind{{0,1,2}};
    // float eye_fov = 120, aspect_ratio = 1, zNear = -1, zFar = -50;
    // Eigen::Vector3f T(-5, 0, -5), S(0.1, 0.5, 0.3), P0(0, -1, 6), P1(0, -3, -7);
    // Eigen::Vector3f axis(0, 1, 0);
    // *******************************************************************************************************************************
    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    

        // std::clog<<"Final is "<<get_projection_matrix(eye_fov,aspect_ratio,zNear,zFar) * get_view_matrix(eye_pos) * get_model_matrix(angle,T,S,P0,P1)<<std::endl;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, T, S, P0, P1));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar));
        r.draw(pos_id, ind_id, line_color, rst::Primitive::Triangle);
        cv::Mat image(1024, 1024, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, T, S, P0, P1));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar));

        r.draw(pos_id, ind_id, line_color, rst::Primitive::Triangle);

        cv::Mat image(1024, 1024, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
        std::clog << "angle: " << angle << std::endl;
    

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
