#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <mpi.h>

#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "./headers/physics.h"
#include "./headers/checkpoint.h"


int n_body;
int n_iteration;


int rank;
int world_size;


double* total_m;
double* total_x;
double* total_y;
double* total_vx;
double* total_vy;


double* local_m;
double* local_x;
double* local_y;
double* local_vx;
double* local_vy;


void generate_data(double *m, double *x,double *y,double *vx,double *vy, int n) {
    // TODO: Generate proper initial position and mass for better visualization
    for (int i = 0; i < n; i++) {
        m[i] = rand() % max_mass + 1.0f;
        x[i] = rand() % bound_x;
        y[i] = rand() % bound_y;
        vx[i] = 0.0f;
        vy[i] = 0.0f;
    }
}

void update_position(double *x, double *y, double *vx, double *vy, int st, int len) {
    //TODO: update position 
    int ed = st + len > n_body ? n_body : st + len;
    for(int i = st; i < ed; i++){
        x[i] += vx[i] * dt;
        y[i] += vy[i] * dt;
        if(x[i] <= error) {
            x[i] = error + radius2;
            vx[i] *= -1;
            vy[i] *= -1;
        }
        if(x[i] >= bound_x - error) {
            x[i] = bound_x - error - radius2;
            vx[i] *= -1;
            vy[i] *= -1;
        }
        if(y[i] <= error) {
            y[i] = error + radius2;
            vy[i] *= -1;
            vx[i] *= -1;
        }
        if(y[i] >= bound_y - error) {
            y[i] = bound_y - error - radius2;
            vy[i] *= -1;
            vx[i] *= -1;
        }
        for(int j = 0; j < n_body; j++)
            if(i != j){
                double dx = x[j] - x[i], dy = y[j] - y[i];
                double distance = sqrt(dx * dx + dy * dy);  
                if(distance < error) {vx[i] *= -1; vy[i] *= -1;}
            }
    }
}

void update_velocity(double *m, double *x, double *y, double *vx, double *vy, int st, int len) {
    //TODO: calculate force and acceleration, update velocity
    int ed = st + len > n_body ? n_body : st + len;
    for(int i = st; i < ed; i++) 
        for(int j = 0; j < n_body; j++)
            if(i != j){
                double dx = x[j] - x[i], dy = y[j] - y[i];
                double distance = sqrt(dx * dx + dy * dy);  
                if(distance < radius2 * 2) continue;
                double a = gravity_const * m[j] / (distance*distance);
                vx[i] += dt * a * dx / distance;
                vy[i] += dt * a * dy / distance;
            }   
    
}


int main(int argc, char *argv[]) {
    n_body = atoi(argv[1]);
    n_iteration = atoi(argv[2]);

    total_m = new double[n_body];
    total_x = new double[n_body];
    total_y = new double[n_body];
    total_vx = new double[n_body];
    total_vy = new double[n_body];
    
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (rank == 0) {
		#ifdef GUI
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowSize(800, 800); 
		glutInitWindowPosition(0, 0);
		glutCreateWindow("N Body Simulation MPI Implementation");
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0, bound_x, 0, bound_y);
		#endif
       
	} 
    int block = (n_body + world_size - 1) / world_size;
    int start = block * rank;
    // Logger l = Logger("mpi", n_body, bound_x, bound_y);

    if (rank == 0){
        generate_data(total_m, total_x, total_y, total_vx, total_vy, n_body);
    }
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
   

    MPI_Bcast(total_m, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    

    for (int i = 0; i < n_iteration; i++){
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        
        MPI_Bcast(total_x, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(total_y, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(total_vx, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(total_vy, n_body, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        update_velocity(total_m, total_x, total_y, total_vx, total_vy, start, block);
        MPI_Barrier(MPI_COMM_WORLD);
        update_position(total_x, total_y, total_vx, total_vy, start, block);
        local_x = new double[block];
        local_y = new double[block];
        local_vx = new double[block];
        local_vy = new double[block];
        int len = start + block > n_body ? n_body - start : block;
        for(int j = 0; j < len; j++){
            local_x[j] = total_x[j + start];
            local_y[j] = total_y[j + start];
            local_vx[j] = total_vx[j + start];
            local_vy[j] = total_vy[j + start];
        }
        MPI_Gather(local_x, len, MPI_DOUBLE, total_x, block, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Gather(local_y, len, MPI_DOUBLE, total_y, block, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Gather(local_vx, len, MPI_DOUBLE, total_vx, block, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Gather(local_vy, len, MPI_DOUBLE, total_vy, block, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        // if(rank == 0)printf("%d %d %f\n", rank, i, total_y[0]);
        delete local_x;
        delete local_y;
        delete local_vx;
        delete local_vy;
        
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = t2 - t1;

        if (rank == 0){
            // l.save_frame(total_x, total_y);

            // printf("Iteration %d, elapsed time: %.3f\n", i, time_span);

            #ifdef GUI
            glClear(GL_COLOR_BUFFER_BIT);
            glColor3f(1.0f, 0.0f, 0.0f);
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            double xi;
            double yi;
            for (int j = 0; j < n_body; j++){
                xi = total_x[j];
                yi = total_y[j];
                glVertex2f(xi, yi);
            }
            glEnd();
            glFlush();
            glutSwapBuffers();
            #else

            #endif
        }
        
    }

    delete total_m;
    delete total_x;
    delete total_y;
    delete total_vx;
    delete total_vy;

	if (rank == 0){
        std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> total_time = t4 - t3;
		printf("Student ID: 120090266\n"); // replace it with your student id
		printf("Name: Feng Yutong\n"); // replace it with your name
		printf("Assignment 3: N Body Simulation MPI Implementation\n");
        printf("Total time of %d iteration is %.3f\n", n_iteration, total_time);

	}

	MPI_Finalize();

	return 0;
}

