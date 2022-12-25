#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <chrono>


int main (int argc, char **argv){

    MPI_Init(&argc, &argv); 

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int num_elements; // number of elements to be sorted
    
    num_elements = atoi(argv[1]); // convert command line argument to num_elements

    int elements[num_elements]; // store elements
    int sorted_elements[num_elements]; // store sorted elements

    if (rank == 0) { // read inputs from file (master process)
        std::ifstream input(argv[2]);
        int element;
        int i = 0;
        while (input >> element) {
            elements[i] = element;
            i++;
        }
        std::cout << "actual number of elements:" << i << std::endl;

        // std::cout<<"Array before sort is:\n";
        // for (int k = 0; k < num_elements; k++) std::cout<<elements[k]<<" ";
        // std::cout<<"\n";
    }

    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;
    std::chrono::duration<double> time_span;
    if (rank == 0){ 
        t1 = std::chrono::high_resolution_clock::now(); // record time
    }

    /* TODO BEGIN
        Implement parallel odd even transposition sort
        Code in this block is not a necessary. 
        Replace it with your own code.
        Useful MPI documentation: https://rookiehpc.github.io/mpi/docs
    */
 
    int num_my_element = num_elements / world_size; // number of elements allocated to each process
    int my_element[num_my_element]; // store elements of each process

    MPI_Scatter(elements, num_my_element, MPI_INT, my_element, num_my_element, MPI_INT, 0, MPI_COMM_WORLD); // distribute elements to each process
    int rd = 0;
    bool fg = 0, tot_flag = 0;
    int a;
    while(!tot_flag){
        fg = 1;
        tot_flag = 1;


        // even-odd
        for(int j = 1; j < num_my_element; j += 2)
            if(my_element[j] < my_element[j - 1]){
                std::swap(my_element[j], my_element[j - 1]);
                fg = 0;
            }

        if (num_my_element & 1){
            if(rank){
                MPI_Send(my_element, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
                MPI_Recv(my_element, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            }
            // std::cout<<rank<<" send\n";
            if(rank < world_size - 1){
                MPI_Recv(&a, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if(a < my_element[num_my_element-1]) {
                    std::swap(a, my_element[num_my_element - 1]);
                    fg = 0;
                }
                MPI_Send(&a, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
            }
        }
        // odd-even
        for(int j = 2; j < num_my_element; j += 2)
            if(my_element[j] < my_element[j - 1]){
                std::swap(my_element[j], my_element[j - 1]);
                fg = 0;
            }

        if (!(num_my_element & 1)){
            if(rank){
                MPI_Send(my_element, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
                MPI_Recv(my_element, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
            }
            // std::cout<<rank<<" send\n";
            if(rank < world_size - 1){
                MPI_Recv(&a, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if(a < my_element[num_my_element-1]) {
                    std::swap(a, my_element[num_my_element - 1]);
                    fg = 0;
                }
                MPI_Send(&a, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
            }
        }

        MPI_Reduce(&fg, &tot_flag, 1, MPI_C_BOOL, MPI_LAND, 0, MPI_COMM_WORLD);
        MPI_Bcast(&tot_flag, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        // std::cout<<"This rank is "<<rank<<" "<<rd<<" "<<fg<<" "<<tot_flag<<"haha\n";
        rd += 1;
        
    } 
    MPI_Gather(my_element, num_my_element, MPI_INT, sorted_elements, num_my_element, MPI_INT, 0, MPI_COMM_WORLD); // collect result from each process
    /* TODO END */

    if (rank == 0){ // record time (only executed in master process)
        t2 = std::chrono::high_resolution_clock::now();  
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        std::cout << "Student ID: " << "120090266" << std::endl; // replace it with your student id
        std::cout << "Name: " << "Feng Yutonng" << std::endl; // replace it with your name
        std::cout << "Assignment 1: Parallel" << std::endl;
        std::cout << "Run Time: " << time_span.count() << " seconds" << std::endl;
        std::cout << "Input Size: " << num_elements << std::endl;
        std::cout << "Process Number: " << world_size << std::endl; 
    }

    if (rank == 0){ // write result to file (only executed in master process)
        // std::cout<<"Array after sort is:\n";
        // for (int k = 0; k < num_elements; k++) 
        // std::cout<<sorted_elements[k]<<" ";
        // std::cout<<"\n";

        std::ofstream output(argv[2]+std::string(".parallel.out"), std::ios_base::out);
        for (int i = 0; i < num_elements; i++) {
            output << sorted_elements[i] << std::endl;
        }
        
    }
    
    MPI_Finalize();
    
    return 0;
}


