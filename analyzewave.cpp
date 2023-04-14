// analyzewave.cpp
//
// Reads a netcdf file produced by wave2d and computes the potential,
// kinetic, and total energy of the wave as a function of time.
//
#include <mpi.h>
#include <fstream>
#include <iostream>
#include <netcdf>
#include <rarray>
#include <chrono>

#include "timing_output.h"

using namespace netCDF;

// helper routines to get attributes and dimensions from an open netcdf file
double get_double_attribute(NcFile& f, const std::string& attname)
{
    double result;
    f.getAtt(attname).getValues(&result);
    return result;
}

unsigned long get_dimension(NcFile& f, const std::string& dimname)
{
    return f.getDim(dimname).getSize();
}

int main(int argc, char** argv)
{   
    // initiate and start the timer
    auto start_time = std::chrono::steady_clock::now();

    // Program parameters are the name of an input file and an output
    // file and are given from command line or, if not, are given
    // default values "precisewave.nc" and "energies.tsv".
    
    std::string inputfilename((argc<=1)?"precisewave.nc":argv[1]);
    std::string outputfilename((argc<=2)?"energies.tsv":argv[2]);
   
    // MPI settings
    int size;
    int rank;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    
    if (rank ==0) {
    std::cout << "Reading from " << inputfilename << "\n";
    std::cout << "Writing to " << outputfilename << "\n";
    }

    // Open file and get some of the simulation parameters as well as the
    // number of stored steps in the netcdf file 
    NcFile f(inputfilename, NcFile::read);
    unsigned long nrows     = get_dimension(f, "nxgrid"); 
    unsigned long ncols     = get_dimension(f, "nygrid"); 
    unsigned long noutsteps = get_dimension(f, "noutsteps"); 
    double c       = get_double_attribute(f, "c");
    double dx      = get_double_attribute(f, "dx");
    double outtime = get_double_attribute(f, "outtime");
    double runtime = get_double_attribute(f, "runtime");
    double time;

    int up = rank-1; if(up<0)up=MPI_PROC_NULL;
    int down = rank+1; if(down>=size)down=MPI_PROC_NULL;
    int localrows = nrows / size + ((rank < nrows % size) ? 1 : 0) ;
    int startrow = nrows / size * rank + std::min(rank, int(nrows % size)) ;
    if (rank==0) {
    std::cout << "Process " << rank << " is reading from " << inputfilename << "\n";
    std::cout << "Process " << rank << " is writing to " << outputfilename << "\n";
    }
    std::cout << "Process " << rank << " starts at row " << startrow << " and ends at row " << startrow + localrows << "\n";

    
    // Find the rho and t variables, and create arrays to hold time slices
    NcVar rho_handle  = f.getVar("rho");
    NcVar time_handle = f.getVar("t");    

    //double a = 0.25*dt/pow(dx,2);
    int guardup = 0;
    int guarddown = localrows + 1;


    // Create arrays for the densities
    rmatrix<double> rho(localrows + 2,ncols);
    rmatrix<double> rho_prev(localrows + 2,ncols);

    // Start output with a header
    std::ofstream fout(outputfilename);
    fout << "# c="         << c << "\n"
         << "# dx="        << dx << "\n"
         << "# outtime="   << outtime << "\n"
         << "# ngrid="     << nrows << "x" << ncols << "\n"
         << "# noutsteps=" << noutsteps << "\n"
         << "#time\tkinetic-energy\tpotential-energy\ttotal-energy\n";

    // Get first time slice
    rho_handle.getVar({0,startrow,0}, {1,localrows,ncols}, &rho[1][0]);
    //rho_prev = rho;
    time_handle.getVar({0}, {1}, &time);
    MPI_Sendrecv(&rho[1][0], 	 ncols, MPI_DOUBLE, up,   11,
    	     &rho[guarddown][0], ncols, MPI_DOUBLE, down, 11,
    	     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Sendrecv(&rho[localrows][0], ncols, MPI_DOUBLE, down, 11,
    	     &rho[guardup][0],   ncols, MPI_DOUBLE, up,   11,
    	     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    if (rank==0) {
    std::cout << "Analyzing total time " << runtime
              << " (" << noutsteps << " steps)\n";
    }

    // set up the timer that will be needed to stop the program prematurely
    //int stop_seconds = std::stoi(argv[3]);
    //std::cout << stop_seconds << std::endl;
    double full_running_time = 0.0;
    auto end_time = std::chrono::steady_clock::now();
    
    for (unsigned long s = 1; s < noutsteps; s++)
    {
        // store previous slice as rho_prev to be able to compute the
        // time derivative of the wave field below
        std::swap(rho, rho_prev);
        
        // get next time slice
        time_handle.getVar({s}, {1}, &time);
        //rho_handle.getVar({s,0,0}, {1,nrows,ncols}, rho.data());
	//rho_handle.getVar({s, rank * localrows, 0}, {1, localrows, ncols}, rho.data());
	rho_handle.getVar({s, startrow, 0}, {1, localrows, ncols}, &rho[1][0]);

	// MPI send/recieve
	MPI_Sendrecv(&rho[1][0], 	 ncols, MPI_DOUBLE, up,   11,
		     &rho[guarddown][0], ncols, MPI_DOUBLE, down, 11,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Sendrecv(&rho[localrows][0], ncols, MPI_DOUBLE, down, 11,
		     &rho[guardup][0],   ncols, MPI_DOUBLE, up,   11,
		     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	if (rank==0) {
        for (int j=1; j<ncols-1; j++) {
	rho[guardup][j]=0.0;
	//rho_prev[0][j]=0.0; 
	}
	}
	if (rank==size-1) {
        for (int j=1; j<ncols-1; j++) {
	rho[guarddown][j]=0.0;
	//rho_prev[localrows+1][j]=0.0;
	}
	}
        
        
        // report status to console
        std::cout << "\rCurrently analyzing time " << time << " (step " << s+1 << ")     ";
        std::cout.flush();

        // compute energies
        double T_local = 0.0;
        double V_local = 0.0;
        for (int i=1; i<localrows+1; i++) {
            for (int j=1; j<ncols-1; j++) {
                T_local += pow((dx/outtime)*(rho[i][j]-rho_prev[i][j]),2);
                V_local += pow(c,2)*(pow(rho[i][j]-rho[i-1][j],2)
                         +pow(rho[i][j]-rho[i][j-1],2));
            }
        }
	//std::cout << "Process " << rank << "\t" << T_local << "\t" << V_local << "\t" << T_local+V_local << "\n";
	
	// reduce local energies to global energies
	double T, V;
	MPI_Reduce(&T_local, &T, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Reduce(&V_local, &V, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        // store in output file
	if (rank == 0) {
		fout << time << "\t" << T << "\t" << V << "\t" << T+V << "\n";
	}
	//if (elapsed_time > stop_seconds) {
	//	break;
	//}
    }

    if (rank == 0) {
	    fout.close();
	    std::cout << "\nDone\nOutput written to " << outputfilename <<"\n";
	    // get the end of the time
	    end_time = std::chrono::steady_clock::now();
	    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	    //std::cout << "Duration " << duration.count() << std::endl;
	    full_running_time = duration.count()/1000;
	    //std::cout << elapsed_time << std::endl;
    	    write_timing(size,full_running_time);

    }

    MPI_Finalize();

}
