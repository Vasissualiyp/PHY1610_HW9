#include <iostream>
#include <cstdlib>
#include <string>

int main() {
    // An array of values for the number of processors
    //int processor_counts[] = {1, 2, 3, 5, 9, 16};
    int processor_counts[] = {4, 8, 12, 14, 16};

    // Iterate through each processor count
    for (int proc_count : processor_counts) {
        // Construct the command string
        std::string command = "mpirun -n " + std::to_string(proc_count) + " ./analyzewave precisewave.nc result.tsv";

        // Execute the command
        int result = std::system(command.c_str());

        // Check the result of the command execution
        if (result != 0) {
            std::cerr << "Error executing command for processor count " << proc_count << std::endl;
            return 1;
        }

        std::cout << "Command executed successfully for processor count " << proc_count << std::endl;
    }

    return 0;
}

