#include "timing_output.h"

// This function is used to store the general data about the performance of the runs (total runtime)
int write_timing(int cores_number, int integer2)
{
    // Read file and find line starting with integer
    const char* filename = "timing_output.csv";
    std::ifstream file(filename);
    //std::ofstream file(filename, std::ios::app);
    std::string line;
    bool found_line = false;
    int integer3 = 0;
    //if (file.is_open()) {
    //    file << cores_number << ", " << integer2 << std::endl;
    //    file.close();
    //}
    //else {
    //    std::cerr << "Unable to open file: " << filename << std::endl;
    //    return 1;
    //}
    if (file.good()) {  // file exists
            while (std::getline(file, line)) {
        	if (std::isdigit(line[0]) && std::isdigit(line[1])) {
        	    int line_cores_number = std::stoi(line.substr(0, 2));
        	    if (line_cores_number == cores_number) {
        		found_line = true;
        		break;
        	    }
        	}
            }
            file.close();
            
            // If line found, delete it
            if (found_line) {
        	std::ifstream file2(filename);
        	std::ofstream temp("temp.txt");
        	while (std::getline(file2, line)) {
        	    if (!(std::isdigit(line[0]) && std::isdigit(line[1]) && 
        		  std::stoi(line.substr(0, 2)) == cores_number)) {
        		temp << line << std::endl;
        	    }
        	}
        	file2.close();
        	temp.close();
        	std::remove(filename);
        	std::rename("temp.txt", filename);
            }
            
            // Append new line to file
            std::ofstream outfile(filename, std::ios_base::app);
            outfile << cores_number << "," << integer2 << "," << integer3 << std::endl;
            outfile.close();
    } else {  // file does not exist
        std::ofstream outfile(filename);
        outfile << "number_of_proccesses,time,dummy_column" << "\n";
        outfile << cores_number << "," << integer2 << "," << integer3 << "\n";
        outfile.close();
    }
    return 0;
}

