
// Version 1: extra space

#include "rasterizer.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

namespace CGL {

  RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
    size_t width, size_t height,
    unsigned int sample_rate) {
    this->psm = psm;
    this->lsm = lsm;
    this->width = width;
    this->height = height;
    this->sample_rate = sample_rate;

    sample_buffer.resize(width * height * sample_rate, Color::White);
    // z_buffer.resize(width * height * sample_rate, numeric_limits<float>::infinity());

  }

  // Used by rasterize_point and rasterize_line
  void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
    // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
    // NOTE: You are not required to implement proper supersampling for points and lines
    // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)
    int size = sqrt(sample_rate);
    for(int ii = 0; ii < size; ii++)
      for(int jj = 0; jj < size; jj++){
        sample_buffer[(y * size + jj) * width * size + x * size + ii] = c;
        // z_buffer[(y * size + jj) * width * size + x * size + ii] = d;
      }
    // cout<<"fill "<<x<<" "<<y<<" "<<c<<endl;
  }

  // Rasterize a point: simple example to help you start familiarizing
  // yourself with the starter code.
  //
  void RasterizerImp::rasterize_point(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= width) return;
    if (sy < 0 || sy >= height) return;

    fill_pixel(sx, sy, color);
    return;
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    if (x0 > x1) {
      swap(x0, x1); swap(y0, y1);
    }

    float pt[] = { x0,y0 };
    float m = (y1 - y0) / (x1 - x0);
    float dpt[] = { 1,m };
    int steep = abs(m) > 1;
    if (steep) {
      dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
      dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
    }

    while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
      rasterize_point(pt[0], pt[1], color);
      pt[0] += dpt[0]; pt[1] += dpt[1];
    }
  }
  
  bool RasterizerImp::inside(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2){
    float epc = -(1e-7);
    float l0 = -(x-x0)*(y1-y0) + (y-y0)*(x1-x0);
    float l1 = -(x-x1)*(y2-y1) + (y-y1)*(x2-x1);
    float l2 = -(x-x2)*(y0-y2) + (y-y2)*(x0-x2);
    if(l0 >= 0 && l1 >= 0 && l2 >= 0) return true;
    if(l0 <= 0 && l1 <= 0 && l2 <= 0) return true;
    return false;
  }
  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color) {
    //   cout<<sample_rate<<"  Task1\n";
    // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
    
    // TODO: Task 2: Update to implement super-sampled rasterization
   
    int size = sqrt(sample_rate);
    float xminf = min(x0, min(x2, x1)), xmaxf = max(x0, max(x2, x1));
    float yminf = min(y0, min(y1, y2)), ymaxf = max(y0, max(y1, y2));
    int xmin = floor(xminf), xmax = ceil(xmaxf), ymax = ceil(ymaxf), ymin = floor(yminf);
    for(int i = xmin; i <= xmax; i++)
    for(int j = ymin; j <= ymax; j++){
        
        for(int ii = 0; ii < size; ii++)
            for(int jj = 0; jj < size; jj++){
            float x = i + (0.5 + ii)/size, y = j + (0.5 + jj)/size;
            // float w_inv = 1.0
            if(inside(x, y, x0, y0, x1, y1, x2, y2))// && z < z_buffer[(j*size + jj) * width * size + i * size + ii]){
                this->sample_buffer[(j*size + jj) * width * size + i * size + ii] = color;
                // this->z_buffer[(j*size + jj) * width * size + i * size + ii] = z;
            //}
        }

    }
      
    
  }
  Vector3D interpolation(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2){
    float alpha = (-(x-x1)*(y2-y1)+(y-y1)*(x2-x1)) / (-(x0-x1)*(y2-y1)+(y0-y1)*(x2-x1));
    float beta = (-(x-x2)*(y0-y2)+(y-y2)*(x0-x2)) / (-(x1-x2)*(y0-y2)+(y1-y2)*(x0-x2));
    float gamma = 1 - alpha - beta;
    return Vector3D(alpha, beta, gamma);
  }

  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle
    // cout<<"Task 4"<<endl;
    
    int size = sqrt(sample_rate);
    float xminf = min(x0, min(x2, x1)), xmaxf = max(x0, max(x2, x1));
    float yminf = min(y0, min(y1, y2)), ymaxf = max(y0, max(y1, y2));
    int xmin = floor(xminf), xmax = ceil(xmaxf), ymax = ceil(ymaxf), ymin = floor(yminf);
    for(int i = xmin; i <= xmax; i++)
    for(int j = ymin; j <= ymax; j++){
        for(int ii = 0; ii < size; ii++)
            for(int jj = 0; jj < size; jj++){
              float x = i + (0.5 + ii)/size, y = j + (0.5 + jj)/size;
            // cout<<i<<" "<<j<<" "<<ii<<" "<<jj<<endl;
              if(inside(x, y, x0, y0, x1, y1, x2, y2)){
                Vector3D para = interpolation(x, y, x0, y0, x1, y1, x2, y2);
                float alpha = para[0], beta = para[1], gamma = para[2];
                Color col = alpha*c0 + beta*c1 + gamma*c2;
                this->sample_buffer[(j*size + jj) * width * size + i * size + ii] = col;
              }
          }
      }

  }


  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle

    int size = sqrt(sample_rate);
    float xminf = min(x0, min(x2, x1)), xmaxf = max(x0, max(x2, x1));
    float yminf = min(y0, min(y1, y2)), ymaxf = max(y0, max(y1, y2));
    int xmin = floor(xminf), xmax = ceil(xmaxf), ymax = ceil(ymaxf), ymin = floor(yminf);
    SampleParams sp = SampleParams {
      Vector2D(), Vector2D(), Vector2D(),
      psm, lsm
    };
    
    for(int i = xmin; i <= xmax; i++)
    for(int j = ymin; j <= ymax; j++){
        for(int ii = 0; ii < size; ii++)
            for(int jj = 0; jj < size; jj++){
              float x = i + (0.5 + ii)/size, y = j + (0.5 + jj)/size;
              if(inside(x, y, x0, y0, x1, y1, x2, y2)){
                Vector3D para = interpolation(x, y, x0, y0, x1, y1, x2, y2);
                float alpha = para[0], beta = para[1], gamma = para[2];
                sp.p_uv = Vector2D(u0*alpha + u1*beta + u2*gamma, v0*alpha + v1*beta + v2*gamma);

                para = interpolation(x+1, y, x0, y0, x1, y1, x2, y2);
                alpha = para[0], beta = para[1], gamma = para[2];
                sp.p_dx_uv = Vector2D(u0*alpha + u1*beta + u2*gamma, v0*alpha + v1*beta + v2*gamma);

                para = interpolation(x, y+1, x0, y0, x1, y1, x2, y2);
                alpha = para[0], beta = para[1], gamma = para[2];
                sp.p_dy_uv = Vector2D(u0*alpha + u1*beta + u2*gamma, v0*alpha + v1*beta + v2*gamma);


                this->sample_buffer[(j*size + jj) * width * size + i * size + ii] = tex.sample(sp);
              }
          }
      }


  }

    void RasterizerImp::set_sample_rate(unsigned int rate) {
        // TODO: Task 2: You may want to update this function for supersampling support

        this->sample_rate = rate;


        this->sample_buffer.resize(width * height * sample_rate, Color::White);
    }


    void RasterizerImp::set_framebuffer_target(unsigned char *rgb_framebuffer,
                                               size_t width, size_t height) {
        // TODO: Task 2: You may want to update this function for supersampling support

        this->width = width;
        this->height = height;
        this->rgb_framebuffer_target = rgb_framebuffer;
        this->sample_buffer.resize(width * height * sample_rate, Color::White);
    }


    void RasterizerImp::clear_buffers() {
        std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
        std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
        // std::fill(z_buffer.begin(), z_buffer.end(), numeric_limits<float>::infinity());

    }


    // This function is called at the end of rasterizing all elements of the
    // SVG file.  If you use a supersample buffer to rasterize SVG elements
    // for antialising, you could use this call to fill the target framebuffer
    // pixels from the supersample buffer data.
    //
    void RasterizerImp::resolve_to_framebuffer() {
        // TODO: Task 2: You will likely want to update this function for supersampling support

        int size = sqrt(sample_rate);
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                Color col = Color(0, 0, 0);
                float d = 0;
                for (int ii = 0; ii < size; ++ii) {
                    for (int jj = 0; jj < size; ++jj) {
                        col += this->sample_buffer[(y * size + jj) * width * size + x * size + ii];
                        // d += this->z_buffer[(y * size + jj) * width * size + x * size + ii];
                    }
                }
                d /= sample_rate;
                for (int k = 0; k < 3; ++k) {
                    this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255 / sample_rate;
                }
            }
        }

    }

    Rasterizer::~Rasterizer() {}


}// CGL


// // TODO: Task 2: don't scale sample_buffer size

// Version 2 no extra space
// #include "rasterizer.h"
// #include <cmath>
// #include <vector>
// #include <algorithm>
// #include <iostream>
// using namespace std;

// namespace CGL {

//   RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
//     size_t width, size_t height,
//     unsigned int sample_rate) {
//     this->psm = psm;
//     this->lsm = lsm;
//     this->width = width;
//     this->height = height;
//     this->sample_rate = sample_rate;

//     sample_buffer.resize(width * height, Color::White);
//     // z_buffer.resize(width * height * sample_rate, numeric_limits<float>::infinity());

//   }

//   // Used by rasterize_point and rasterize_line
//   void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
//     // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
//     // NOTE: You are not required to implement proper supersampling for points and lines
//     // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)

//         sample_buffer[y * width + x] = c;

//   }

//   // Rasterize a point: simple example to help you start familiarizing
//   // yourself with the starter code.
//   //
//   void RasterizerImp::rasterize_point(float x, float y, Color color) {
//     // fill in the nearest pixel
//     int sx = (int)floor(x);
//     int sy = (int)floor(y);

//     // check bounds
//     if (sx < 0 || sx >= width) return;
//     if (sy < 0 || sy >= height) return;

//     fill_pixel(sx, sy, color);
//     return;
//   }

//   // Rasterize a line.
//   void RasterizerImp::rasterize_line(float x0, float y0,
//     float x1, float y1,
//     Color color) {
//     if (x0 > x1) {
//       swap(x0, x1); swap(y0, y1);
//     }

//     float pt[] = { x0,y0 };
//     float m = (y1 - y0) / (x1 - x0);
//     float dpt[] = { 1,m };
//     int steep = abs(m) > 1;
//     if (steep) {
//       dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
//       dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
//     }

//     while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
//       rasterize_point(pt[0], pt[1], color);
//       pt[0] += dpt[0]; pt[1] += dpt[1];
//     }
//   }
  
//   bool RasterizerImp::inside(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2){
//     float epc = -(1e-7);
//     float l0 = -(x-x0)*(y1-y0) + (y-y0)*(x1-x0);
//     float l1 = -(x-x1)*(y2-y1) + (y-y1)*(x2-x1);
//     float l2 = -(x-x2)*(y0-y2) + (y-y2)*(x0-x2);
//     if(l0 >= 0 && l1 >= 0 && l2 >= 0) return true;
//     if(l0 <= 0 && l1 <= 0 && l2 <= 0) return true;
//     return false;
//   }
//   // Rasterize a triangle.
//   void RasterizerImp::rasterize_triangle(float x0, float y0,
//     float x1, float y1,
//     float x2, float y2,
//     Color color) {
//     //   cout<<sample_rate<<"  Task1\n";
//     // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
    
//     // TODO: Task 2: Update to implement super-sampled rasterization
//     for(int k = 0; k < 3; k++){
//     int size = sqrt(sample_rate);
//     float xminf = min(x0, min(x2, x1)), xmaxf = max(x0, max(x2, x1));
//     float yminf = min(y0, min(y1, y2)), ymaxf = max(y0, max(y1, y2));
//     int xmin = floor(xminf), xmax = ceil(xmaxf), ymax = ceil(ymaxf), ymin = floor(yminf);
//     for(int i = xmin; i <= xmax; i++)
//     for(int j = ymin; j <= ymax; j++){
//         int cnt = 0;
//         for(int ii = 0; ii < size; ii++)
//             for(int jj = 0; jj < size; jj++){
//             float x = i + (0.5 + ii)/size, y = j + (0.5 + jj)/size;
//             // float w_inv = 1.0
//             if(inside(x, y, x0, y0, x1, y1, x2, y2)) cnt++;// && z < z_buffer[(j*size + jj) * width * size + i * size + ii]){
//         }
//         if(cnt ){
//           Color col = 1.0*cnt / sample_rate * color + 1.0*(sample_rate - cnt) / sample_rate * sample_buffer[j * width + i] ;
//           // if(cnt != sample_rate)cout<<color<<"  "<<sample_buffer[j*width+i]<<" "<<col<<endl;
//           sample_buffer[j * width + i] = col;
//         }

//     }
//     }
    
//   }
//   Vector3D interpolation(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2){
//     float alpha = (-(x-x1)*(y2-y1)+(y-y1)*(x2-x1)) / (-(x0-x1)*(y2-y1)+(y0-y1)*(x2-x1));
//     float beta = (-(x-x2)*(y0-y2)+(y-y2)*(x0-x2)) / (-(x1-x2)*(y0-y2)+(y1-y2)*(x0-x2));
//     float gamma = 1 - alpha - beta;
//     return Vector3D(alpha, beta, gamma);
//   }

//   void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
//     float x1, float y1, Color c1,
//     float x2, float y2, Color c2)
//   {
//     // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
//     // Hint: You can reuse code from rasterize_triangle
//     // cout<<"Task 4"<<endl;
    
//     int size = sqrt(sample_rate);
//     float xminf = min(x0, min(x2, x1)), xmaxf = max(x0, max(x2, x1));
//     float yminf = min(y0, min(y1, y2)), ymaxf = max(y0, max(y1, y2));
//     int xmin = floor(xminf), xmax = ceil(xmaxf), ymax = ceil(ymaxf), ymin = floor(yminf);
//     for(int i = xmin; i <= xmax; i++)
//     for(int j = ymin; j <= ymax; j++){

//               float x = i + 0.5, y = j + 0.5;
//             // cout<<i<<" "<<j<<" "<<ii<<" "<<jj<<endl;
//               if(inside(x, y, x0, y0, x1, y1, x2, y2)){
//                 Vector3D para = interpolation(x, y, x0, y0, x1, y1, x2, y2);
//                 float alpha = para[0], beta = para[1], gamma = para[2];
//                 Color col = alpha*c0 + beta*c1 + gamma*c2;
//                 this->sample_buffer[j*width + i] = col;
//               }
          
//       }

//   }


//   void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
//     float x1, float y1, float u1, float v1,
//     float x2, float y2, float u2, float v2,
//     Texture& tex)
//   {
//     // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
//     // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
//     // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle

//     int size = sqrt(sample_rate);
//     float xminf = min(x0, min(x2, x1)), xmaxf = max(x0, max(x2, x1));
//     float yminf = min(y0, min(y1, y2)), ymaxf = max(y0, max(y1, y2));
//     int xmin = floor(xminf), xmax = ceil(xmaxf), ymax = ceil(ymaxf), ymin = floor(yminf);
//     SampleParams sp = SampleParams {
//       Vector2D(), Vector2D(), Vector2D(),
//       psm, lsm
//     };
    
//     for(int i = xmin; i <= xmax; i++)
//     for(int j = ymin; j <= ymax; j++){
        
//               float x = i + 0.5, y = j + 0.5;
//               if(inside(x, y, x0, y0, x1, y1, x2, y2)){
//                 Vector3D para = interpolation(x, y, x0, y0, x1, y1, x2, y2);
//                 float alpha = para[0], beta = para[1], gamma = para[2];
//                 sp.p_uv = Vector2D(u0*alpha + u1*beta + u2*gamma, v0*alpha + v1*beta + v2*gamma);

//                 para = interpolation(x+1, y, x0, y0, x1, y1, x2, y2);
//                 alpha = para[0], beta = para[1], gamma = para[2];
//                 sp.p_dx_uv = Vector2D(u0*alpha + u1*beta + u2*gamma, v0*alpha + v1*beta + v2*gamma);

//                 para = interpolation(x, y+1, x0, y0, x1, y1, x2, y2);
//                 alpha = para[0], beta = para[1], gamma = para[2];
//                 sp.p_dy_uv = Vector2D(u0*alpha + u1*beta + u2*gamma, v0*alpha + v1*beta + v2*gamma);


//                 this->sample_buffer[j * width + i] = tex.sample(sp);
//               }
          
//       }


//   }

//     void RasterizerImp::set_sample_rate(unsigned int rate) {
//         // TODO: Task 2: You may want to update this function for supersampling support

//         this->sample_rate = rate;


//         this->sample_buffer.resize(width * height, Color::White);
//     }


//     void RasterizerImp::set_framebuffer_target(unsigned char *rgb_framebuffer,
//                                                size_t width, size_t height) {
//         // TODO: Task 2: You may want to update this function for supersampling support

//         this->width = width;
//         this->height = height;
//         this->rgb_framebuffer_target = rgb_framebuffer;
//                 this->sample_buffer.resize(width * height, Color::White);

//     }


//     void RasterizerImp::clear_buffers() {
//         std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
//         std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);

//     }


//     // This function is called at the end of rasterizing all elements of the
//     // SVG file.  If you use a supersample buffer to rasterize SVG elements
//     // for antialising, you could use this call to fill the target framebuffer
//     // pixels from the supersample buffer data.
//     //
//     void RasterizerImp::resolve_to_framebuffer() {
//         // TODO: Task 2: You will likely want to update this function for supersampling support

//         int size = sqrt(sample_rate);
//         for (int x = 0; x < width; ++x) {
//             for (int y = 0; y < height; ++y) {
//                 Color col = sample_buffer[y * width + x];
//                 for (int k = 0; k < 3; ++k) {
//                     this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255;
//                 }
//             }
//         }

//     }

//     Rasterizer::~Rasterizer() {}


// }// CGL


// // TODO: Task 2: dont scale sample_buffer size
