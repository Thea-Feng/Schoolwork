#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <mpi.h>
#include <omp.h>
#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "./headers/physics.h"

int n_omp_threads;
int size; // problem size


int rank;
int world_size;
const int max_iter = 5e3;
float* local_data;
void initialize(float *data) {
    // intialize the temperature distribution
    int len = size * size;
    for (int i = 0; i < len; i++) {
        data[i] = wall_temp;
    }
    return ;
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
    return ;
}


void update(float *data, float *new_data, int begin, int end) {
    // TODO: update the temperature of each point, and store the result in `new_data` to avoid data racing
    begin = begin ? begin : 1;
    end = end > size - 1 ? size - 1 : end;
    omp_set_num_threads(n_omp_threads);
    #pragma omp parallel for
    for(int i = begin; i < end; i++){
       
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
    return;  
}


void maintain_fire(float *data, bool* fire_area, int begin, int end) {
    // TODO: maintain the temperature of fire
    begin = begin ? begin : 1;
    end = end > size - 1 ? size - 1 : end;
    omp_set_num_threads(n_omp_threads);
    #pragma omp parallel for
    for(int i = begin; i < end; i++){
        
        for (int j = 1; j < size - 1; j++){
            int idx = i * size + j;
            if(fire_area[idx]) data[idx] = fire_temp;
        }
    }
    
}

bool check_continue(float *data, float *new_data, int begin, int end){
    bool ret = false;
    // TODO: determine if we should stop (because the temperature distribution will finally converge)
    begin = begin ? begin : 1;
    end = end > size - 1 ? size - 1 : end;
     omp_set_num_threads(n_omp_threads);
        #pragma omp parallel for
    for(int i = begin; i < end; i++){

       
        for (int j = 1; j < size - 1; j++){
            int idx = i * size + j;
            if(abs(data[idx] - new_data[idx]) > threshold) ret = true;
        }
    }
       
    return ret;
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





int main(int argc, char *argv[]) {
    size = atoi(argv[1]);
    n_omp_threads = atoi(argv[2]);


    float* data_odd = new float[size * size];
    float* data_even = new float[size * size];
    bool* fire_area = new bool[size * size];
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int block = (size + world_size - 1) / world_size;


	if (rank == 0) {
        #ifdef GUI
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
        glutInitWindowPosition(0, 0);
        glutInitWindowSize(resolution, resolution);
        glutCreateWindow("Heat Distribution Simulation MPI + Openmp Implementation");
        gluOrtho2D(0, resolution, 0, resolution);
        #endif
        initialize(data_odd);
        generate_fire_area(fire_area);
	} 

    

    #ifdef GUI
    GLubyte* pixels;
    pixels = new GLubyte[resolution * resolution * 3];
    #endif

    int count = 1;
    bool global_cont = true, local_cont = true;
    double total_time = 0;

    // TODO: Send initial distribution to each slave process
    int begin = block * rank;
    int end = rank == world_size - 1 ? size: begin + block; 
    MPI_Bcast(fire_area, size * size, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(data_even, size * size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(data_odd, size * size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    float *begin_block, *end_block;
    begin_block = new float[size * world_size];
    end_block = new float[size * world_size];
    while (global_cont && count < max_iter) {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        // TODO: Computation of my part
        // TODO: Send border row to neighbours
        MPI_Bcast(begin_block, size * world_size, MPI_FLOAT,0, MPI_COMM_WORLD );
        MPI_Bcast(end_block, size * world_size, MPI_FLOAT,0, MPI_COMM_WORLD );
        // MPI_Bcast(data_even, size * size, MPI_FLOAT, 0, MPI_COMM_WORLD);
        // MPI_Bcast(data_odd, size * size, MPI_FLOAT, 0, MPI_COMM_WORLD);
        
        
        if (count % 2 == 1) {
            if(count != 1) {
                if(rank){
                    int st = begin * size - size;
                    for(int i = 0; i < size; i++){
                        data_odd[st + i] = begin_block[(rank - 1) * size + i];
                    }
                }
                if(rank != world_size - 1){
                    int ed = end * size;
                    for(int i = 0; i < size; i++)
                        data_odd[ed + i] = end_block[rank * size + i];
                    
                }
            }
            
                
            update(data_odd, data_even, begin, end);
            maintain_fire(data_even, fire_area, begin, end);
            local_cont = check_continue(data_odd, data_even, begin, end);
        } else {
            if(rank){
                int st = begin * size - size;
                for(int i = 0; i < size; i++){
                    data_even[st + i] = begin_block[(rank - 1) * size + i];
                }
            }
            if(rank != world_size - 1){
                int ed = end * size;
                for(int i = 0; i < size; i++)
                    data_even[ed + i] = end_block[rank * size + i];
            }

            update(data_even, data_odd, begin, end);
            maintain_fire(data_odd, fire_area, begin, end);
            local_cont = check_continue(data_odd, data_even, begin, end);
        }
        int tsize = (end - begin) * size;
        local_data = new float[tsize];
        int idx = 0;
        for(int i = begin; i < end; i++)
            for(int j = 0; j < size; j++)
                local_data[idx++] = count % 2 ? data_even[i * size + j] : data_odd[i * size + j];
        if(count % 2) MPI_Gather(local_data, tsize, MPI_FLOAT, data_even, tsize, MPI_FLOAT, 0, MPI_COMM_WORLD);
        else MPI_Gather(local_data, tsize, MPI_FLOAT, data_odd, tsize, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_cont, &global_cont, 1, MPI_C_BOOL, MPI_LOR, 0, MPI_COMM_WORLD);
        MPI_Bcast(&global_cont, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        delete[] local_data;
        if(rank == 0) {
            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
            double this_time = std::chrono::duration<double>(t2 - t1).count();
            total_time += this_time;
            printf("Iteration %d, elapsed time: %.6f\n", count, this_time);
            
        if(count % 2) {
            int cnt = 0;
            for(int i = 1; i < world_size; i++) {
                int st = i * block * size - size;
                int ed = i * block * size;
                for(int j = 0; j < size; j++) {
                    begin_block[cnt] = data_even[st + j];
                    end_block[cnt++] = data_even[ed + j];
                }
            }
            // printf("%d %d\n", cnt, world_size * size - size);
        }
        else {
            int cnt = 0;
            for(int i = 1; i < world_size; i++) {
                int st = i * block * size - size;
                int ed = i * block * size;
                for(int j = 0; j < size; j++) {
                    begin_block[cnt] = data_odd[st + j];
                    end_block[cnt++] = data_odd[ed + j];
                }
            }
        }
        
            #ifdef GUI
            if (count % 2 == 1) {
                // TODO: Gather pixels of slave processes
                data2pixels(data_even, pixels);
            } else {
                // TODO: Gather pixels of slave processes
                data2pixels(data_odd, pixels);
            }
            plot(pixels);
            #endif
        }
        count++;

        
    }

    delete[] begin_block;
    delete[] end_block;
    #ifdef GUI
    delete[] pixels;
    #endif
	if (rank == 0){
		printf("Student ID: 120090266\n"); // replace it with your student id
		printf("Name: Feng Yutong\n"); // replace it with your name
		printf("Assignment 4: Heat Distribution Simulation MPI + Openmp Implementation\n");
        printf("Converge after %d iterations, elapsed time: %.6f, average computation time: %.6f\n", count-1, total_time, (double) total_time / (count-1));
	}

	MPI_Finalize();

	return 0;
}

