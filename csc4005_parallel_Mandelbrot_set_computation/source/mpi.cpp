#include "asg2.h"
#include <stdio.h>
#include <mpi.h>


int rank;
int world_size;


int main(int argc, char *argv[]) {
	if ( argc == 4 ) {
		X_RESN = atoi(argv[1]);
		Y_RESN = atoi(argv[2]);
		max_iteration = atoi(argv[3]);
	} else {
		X_RESN = 1000;
		Y_RESN = 1000;
		max_iteration = 100;
	}

	if (rank == 0) {
		#ifdef GUI
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowSize(500, 500); 
		glutInitWindowPosition(0, 0);
		glutCreateWindow("MPI");
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0, X_RESN, 0, Y_RESN);
		glutDisplayFunc(plot);
		#endif
	}

	/* computation part begin */
	struct pointtype b;
	MPI_Datatype myvar;
	MPI_Datatype old_types[2];
	MPI_Aint indices[2];
	int blocklens[2];

	MPI_Init(&argc, &argv);

	blocklens[0] = 2;
	blocklens[1] = 1;
	old_types[0] = MPI_INT;
	old_types[1] = MPI_FLOAT;
	MPI_Address(&b, &indices[0]);
	MPI_Address(&b.color, &indices[1]);
	indices[1] -= indices[0];
	indices[0] = 0;
	MPI_Type_struct(2,blocklens,indices,old_types,&myvar);
	MPI_Type_commit(&myvar);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);


	if (rank == 0) {
		t1 = std::chrono::high_resolution_clock::now();
		initData();	
	}
	MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int block = total_size / world_size;
	//printf("%d %d %d\n",block, total_size, world_size);
	Point*  mydata;
	mydata = new Point[block];
	MPI_Scatter(data, block, myvar, mydata, block, myvar, 0, MPI_COMM_WORLD); // distribute elements to each process
	
	MPI_Gather(mydata, block, myvar, data, block, myvar, 0, MPI_COMM_WORLD);
	
	MPI_Finalize();
	/* computation part end */

	if (rank == 0){

		t2 = std::chrono::high_resolution_clock::now();  
		time_span = t2 - t1;

		printf("Student ID: 120090266\n"); // replace it with your student id
		printf("Name: FengYutong\n"); // replace it with your name
		printf("Assignment 2 MPI\n");
		printf("Run Time: %f seconds\n", time_span.count());
		printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
		printf("Process Number: %d\n", world_size);


		#ifdef GUI
		glutMainLoop();
		#endif
	}

	return 0;
}

