#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <fstream>
#include <iostream>
#include <cstdlib>

TEST_CASE("Integrated test of analyzewave") {

    // Get the time to run the test for from the command-line argument
    //int time_to_run = std::atoi(argv[1]);
    // For now, just set the time to run (in seconds)
    int time_to_run = 100;
    int no_proccesses = 4;

    // Run the sim with test wave parameters file
    //std::string command = "./analyzewave precisewave.nc int_test.tsv ";
    std::string command = "mpirun -np "  + std::to_string(no_proccesses) + " ./analyzewave precisewave.nc int_test.tsv ";
    command += std::to_string(time_to_run);
    std::system(command.c_str());
    
    // Compare the output file with the expected file
    std::ifstream expected_file("energies.tsv");
    std::ifstream output_file("int_test.tsv");

    if (!output_file) {
    std::cout << "Error: results file not generated" << std::endl;
    }

    std::string expected_line, output_line;
    //std::stringstream expected_file, output_file;

    while (std::getline(expected_file, expected_line) && std::getline(output_file, output_line)) {
        if (expected_line.at(0) == '#') continue; // Skip comment lines
        REQUIRE(expected_line == output_line);
    }
//    expected_stream << expected_file.rdbuf();
//    output_stream << output_file.rdbuf();
//
//    std::string expected_str = expected_stream.str();
//    std::string output_str = output_stream.str();
//    std::size_t pos = expected_str.find("# c=1");
//    expected_str.erase(0, pos);
//    pos = output_str.find("# c=1");
//    output_str.erase(0, pos);
//
//    REQUIRE(expected_str == output_str);
//

}
