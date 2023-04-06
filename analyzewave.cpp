// analyzewave.cpp
//
// Reads a netcdf file produced by wave2d and computes the potential,
// kinetic, and total energy of the wave as a function of time.
//

#include <fstream>
#include <iostream>
#include <netcdf>
#include <rarray>
#include <chrono>

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
    // Setting up the MPI
    //MPI_Init(&argc,&argv);
    //MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    //MPI_Comm_size(MPI_COMM_WORLD,&size);
    //left = rank-1; if(left<0)left=MPI_PROC_NULL;
    //right = rank+1; if(right>=size)right=MPI_PROC_NULL;
    //localn = n/size;

    // Program parameters are the name of an input file and an output
    // file and are given from command line or, if not, are given
    // default values "precisewave.nc" and "energies.tsv".
    
    std::string inputfilename((argc<=1)?"precisewave.nc":argv[1]);
    std::string outputfilename((argc<=2)?"energies.tsv":argv[2]);
   
    std::cout << "Reading from " << inputfilename << "\n";
    std::cout << "Writing to " << outputfilename << "\n";

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

    // Find the rho and t variables, and create arrays to hold time slices
    NcVar rho_handle  = f.getVar("rho");
    NcVar time_handle = f.getVar("t");    
    rmatrix<double> rho(nrows,ncols);
    rmatrix<double> rho_prev(nrows,ncols);
    double time;

    // Start output with a header
    std::ofstream fout(outputfilename);
    fout << "# c="         << c << "\n"
         << "# dx="        << dx << "\n"
         << "# outtime="   << outtime << "\n"
         << "# ngrid="     << nrows << "x" << ncols << "\n"
         << "# noutsteps=" << noutsteps << "\n"
         << "#time\tkinetic-energy\tpotential-energy\ttotal-energy\n";

    // Get first time slice
    rho_handle.getVar({0,0,0}, {1,nrows,ncols}, rho.data());
    time_handle.getVar({0}, {1}, &time);
    
    std::cout << "Analyzing total time " << runtime
              << " (" << noutsteps << " steps)\n";

    // initiate the timer that will be needed to stop the program prematurely
    int stop_seconds = std::stoi(argv[3]);
    std::cout << stop_seconds << std::endl;
    double elapsed_time = 0.0;
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = std::chrono::steady_clock::now();

    // start the timer for the main loop
    start_time = std::chrono::steady_clock::now();

    for (unsigned long s = 1; s < noutsteps; s++)
    {
        // store previous slice as rho_prev to be able to compute the
        // time derivative of the wave field below
        std::swap(rho, rho_prev);
        
        // get next time slice
        time_handle.getVar({s}, {1}, &time);
        rho_handle.getVar({s,0,0}, {1,nrows,ncols}, rho.data());
        
        // report status to console
        std::cout << "Currently analyzing time " << time << " (step " << s+1 << ")    ";
        std::cout.flush();
        
        // compute energies
        double T = 0.0;
        double V = 0.0;
        for (int i=1; i<nrows-1; i++) {
            for (int j=1; j<ncols-1; j++) {
                T += pow((dx/outtime)*(rho[i][j]-rho_prev[i][j]),2);
                V += pow(c,2)*(pow(rho[i][j]-rho[i-1][j],2)
                    +pow(rho[i][j]-rho[i][j-1],2));
            }
        }
	// get the end of the time
	end_time = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "Duration " << duration.count() << std::endl;
	elapsed_time += duration.count()/1000;
	std::cout << elapsed_time << std::endl;

        // store in output file
        fout << time << "\t" << T << "\t" << V << "\t" << T+V << "\n";

	if (elapsed_time > stop_seconds) {
		break;
        }
    }
    
    f.close();

    std::cout << "\nDone\nOutput written to energies.dat\n";


}
