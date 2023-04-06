#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <fstream>
#include <iostream>

TEST_CASE("Integrated test of analyzewave") {
    
    // Run the sim with test wave parameters file
    std::system("./analyzewave precisewave.nc int_test.tsv 10");

    // Compare the output file with the expected file
    std::ifstream expected_file("energies.tsv");
    std::ifstream output_file("int_test.tsv");

    if (!output_file) {
    std::cout << "Error: results.dat file not generated" << std::endl;
    }

    std::stringstream expected_stream, output_stream;
    expected_stream << expected_file.rdbuf();
    output_stream << output_file.rdbuf();

    std::string expected_str = expected_stream.str();
    std::string output_str = output_stream.str();
    std::size_t pos = expected_str.find("# t = 100\n");
    expected_str.erase(0, pos);
    pos = output_str.find("# t = 100\n");
    output_str.erase(0, pos);

    REQUIRE(expected_str == output_str);


}
