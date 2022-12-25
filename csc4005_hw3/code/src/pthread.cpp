#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif


#include "./headers/physics.h"

const int max_iter = 5000;
int n_thd;
int size; // problem size
static pthread_barrier_t barrier;
float *data_odd;
float *data_even;
bool *fire_area;
bool *cont;
void initialize(float *data) {
    // intialize the temperature distribution
    int len = size * size;
    for (int i = 0; i < len; i++) {
        data[i] = wall_temp;
    }
}


void generate_fire_area(bool *fire_area){
    // generate the fire area
    int len = size * size;
    for (int i = 0; i < len; i++) {
        fire_area[i] = 0;
    }

    float fire1_r2 = fire_size * fire_size;
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            int a = i - size / 2;
            int b = j - size / 2;
            int r2 = 0.5 * a * a + 0.8 * b * b - 0.5 * a * b;
            if (r2 < fire1_r2) fire_area[i * size + j] = 1;
        }
    }

    float fire2_r2 = (fire_size / 2) * (fire_size / 2);
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            int a = i - 1 * size / 3;
            int b = j - 1 * size / 3;
            int r2 = a * a + b * b;
            if (r2 < fire2_r2) fire_area[i * size + j] = 1;
        }
    }
}



void update(float *data, float *new_data, int begin, int end) {
    // TODO: update the temperature of each point, and store the result in `new_data` to avoid data racing
    begin = begin ? begin : 1;
    end = end > size - 1 ? size - 1 : end;
    for(int i = begin; i < end; i++)
        for(int j = 1; j < size - 1; j++) {
            int idx = i * size + j;
            float up = data[idx - size];
            float down = data[idx + size];
            float left = data[idx - 1];
            float right = data[idx + 1];
            float new_val = (up + down + left + right) / 4;
            new_data[idx] = new_val;
        }
}


void maintain_fire(float *data, bool* fire_area, int begin, int end) {
    // TODO: maintain the temperature of fire
    begin = begin ? begin : 1;
    end = end > size - 1 ? size - 1 : end;
    for(int i = begin; i < end; i++)
        for (int j = 1; j < size - 1; j++){
            int idx = i * size + j;
            if(fire_area[idx]) data[idx] = fire_temp;
        }
}

bool check_continue(float *data, float *new_data, int begin, int end){
    // TODO: determine if we should stop (because the temperature distribution will finally converge)
    begin = begin ? begin : 1;
    end = end > size - 1 ? size - 1 : end;
    for(int i = begin; i < end; i++)
        for (int j = 1; j < size - 1; j++){
            int idx = i * size + j;
            if(abs(data[idx] - new_data[idx]) > threshold) return true;
        }
    return false;
}


#ifdef GUI
void data2pixels(float *data, GLubyte* pixels){
    // convert rawdata (large, size^2) to pixels (small, resolution^2) for faster rendering speed
    float factor_data_pixel = (float) size / resolution;
    float factor_temp_color = (float) 255 / fire_temp;
    for (int x = 0; x < resolution; x++){
        for (int y = 0; y < resolution; y++){
            int idx = x * resolution + y;
            int idx_pixel = idx * 3;
            int x_raw = x * factor_data_pixel;
            int y_raw = y * factor_data_pixel;
            int idx_raw = y_raw * size + x_raw;
            float temp = data[idx_raw];
            int color =  ((int) temp / 5 * 5) * factor_temp_color;
            pixels[idx_pixel] = color;
            pixels[idx_pixel + 1] = 255 - color;
            pixels[idx_pixel + 2] = 255 - color;
        }
    }
}


void plot(GLubyte* pixels){
    // visualize temprature distribution
    #ifdef GUI
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(resolution, resolution, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glutSwapBuffers();
    #endif
}
#endif

typedef struct {
    int a, b;
} Args;
void* worker(void *args) {
    Args* my_arg = (Args*) args;
    int a = my_arg->a;
    int count = my_arg->b;
    // int b = my_arg->b;
    
    int block = (size + n_thd - 1) / n_thd;
    int start = block * a;
    int end = start + block > size ? size : start + block; 
    if (count % 2) {
        update(data_odd, data_even, start, end);
        maintain_fire(data_even, fire_area, start, end);
        cont[a] = check_continue(data_odd, data_even, start, end);
    } else {
        update(data_even, data_odd, start, end);
        maintain_fire(data_odd, fire_area, start, end);
        cont[a] = check_continue(data_odd, data_even, start, end);
    }
    // printf("%d %d %d\n",a,b,block);
    // pthread_barrier_wait(&barrier);
    pthread_exit(NULL);
}

void master(){



    #ifdef GUI
    GLubyte* pixels = new GLubyte[resolution * resolution * 3];
    #endif
    
    data_odd = new float[size * size];
    data_even = new float[size * size];
    cont = new bool[n_thd];
    fire_area = new bool[size * size];

    generate_fire_area(fire_area);
    initialize(data_odd);

    int count = 1;
    double total_time = 0;
    Args args[n_thd]; // arguments for all threads
    pthread_t thds[n_thd];
    // pthread_barrier_init(&barrier, NULL, n_thd);
    for (int thd = 0; thd < n_thd; thd++)
        args[thd].a = thd;
    while (count < max_iter) {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        
        for (int thd = 0; thd < n_thd; thd++) {
            args[thd].b = count;
            pthread_create(&thds[thd], NULL, worker, &args[thd]);
        }
        bool total_cont = false;
        for (int thd = 0; thd < n_thd; thd++) {
            pthread_join(thds[thd], NULL);
            total_cont |= cont[thd];
        }
        
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        double this_time = std::chrono::duration<double>(t2 - t1).count();
        total_time += this_time;
        printf("Iteration %d, elapsed time: %.6f\n", count, this_time);
        count++;
        
        #ifdef GUI
        if (count % 2 == 1) {
            data2pixels(data_even, pixels);
        } else {
            data2pixels(data_odd, pixels);
        }
        plot(pixels);
        #endif
        if(!total_cont) break;
    }

    printf("Converge after %d iterations, elapsed time: %.6f, average computation time: %.6f\n", count-1, total_time, (double) total_time / (count-1));

    delete[] data_odd;
    delete[] data_even;
    delete[] fire_area;
    delete[] cont;

    #ifdef GUI
    delete[] pixels;
    #endif
  
}


int main(int argc, char* argv[]) {
    size = atoi(argv[1]);
    n_thd = atoi(argv[2]);
    #ifdef GUI
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(resolution, resolution);
    glutCreateWindow("Heat Distribution Simulation Pthread Implementation");
    gluOrtho2D(0, resolution, 0, resolution);
    #endif

    master();

    printf("Student ID: 120090266\n"); // replace it with your student id
    printf("Name: Feng Yutong\n"); // replace it with your name
    printf("Assignment 4: Heat Distribution Pthread Implementation\n");

    return 0;
}
