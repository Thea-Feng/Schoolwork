#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#ifdef GUI
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "./headers/physics.h"
#include "./headers/checkpoint.h"


int n_body;
int n_iteration;


double* m;
double* x;
double* y;
double* vx;
double* vy;


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



void update_position(double *x, double *y, double *vx, double *vy, int n) {
    //TODO: update position 

    for(int i = 0; i < n; i++){
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
        for(int j = 0; j < n; j++)
            if(i != j){
                double dx = x[j] - x[i], dy = y[j] - y[i];
                double distance = sqrt(dx * dx + dy * dy);  
                if(distance < error) {vx[i] *= -1; vy[i] *= -1;}
            }
    }
}

void update_velocity(double *m, double *x, double *y, double *vx, double *vy, int n) {
    //TODO: calculate force and acceleration, update velocity
    for(int i = 0; i < n; i++) 
        for(int j = 0; j < n; j++)
            if(i != j){
                double dx = x[j] - x[i], dy = y[j] - y[i];
                double distance = sqrt(dx * dx + dy * dy);  
                if(distance < radius2*2) continue;
                double a = gravity_const * m[j] / (distance*distance);
                vx[i] += dt * a * dx / distance;
                vy[i] += dt * a * dy / distance;
            }   

}
std::chrono::high_resolution_clock::time_point t3;


void master() {
    m = new double[n_body];
    x = new double[n_body];
    y = new double[n_body];
    vx = new double[n_body];
    vy = new double[n_body];

    generate_data(m, x, y, vx, vy, n_body);

    // Logger l = Logger("sequential", n_body, bound_x, bound_y);
    t3 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n_iteration; i++){
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        // printf("%d x: %f y: %f vx %f vy %f\n", i, x[98], y[98], vx[98], vy[98]);
        update_velocity(m, x, y, vx, vy, n_body);
        update_position(x, y, vx, vy, n_body);
        // l.save_frame(x, y);


        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = t2 - t1;

        // printf("Iteration %d, elapsed time: %.3f\n", i, time_span);

        #ifdef GUI
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        double xi;
        double yi;
        for (int i = 0; i < n_body; i++){
            xi = x[i];
            yi = y[i];
            glVertex2f(xi, yi);
        }
        glEnd();
        glFlush();
        glutSwapBuffers();
        #else

        #endif
    }

    delete m;
    delete x;
    delete y;
    delete vx;
    delete vy;
    
}


int main(int argc, char *argv[]){
    
    n_body = atoi(argv[1]);
    n_iteration = atoi(argv[2]);

    #ifdef GUI
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("N Body Simulation Sequential Implementation");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gluOrtho2D(0, bound_x, 0, bound_y);
    #endif
    master();
    std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_time = t4 - t3;

    printf("Student ID: 120090266\n"); // replace it with your student id
    printf("Name: Feng Yutong\n"); // replace it with your name
    printf("Assignment 2: N Body Simulation Sequential Implementation\n");
    printf("Total time of %d iteration is %.3f\n", n_iteration, total_time);
    
    return 0;

}
