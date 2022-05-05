#include <iostream>
#include <string>
#include <vector>
#include <sstream>


std::vector<std::string> nrsh_get_line () {

    std::vector<std::string> output;
    std::string S, T;  // declare string variables

    std::getline(std::cin, S); // use getline() function to read a line of string and store into S variable.

    std::stringstream X(S); // X is an object of stringstream that references the S string

    // use while loop to check the getline() function condition
    while (std::getline(X, T, ' ')) {
        /* X represents to read the string from stringstream, T use for store the token string and,
         ' ' whitespace represents to split the string where whitespace is found. */
        output.push_back(T);
        //cout << T << endl; // print split string
    }

    return output;
}
void nrsh_loop(void) {

    std::vector<std::string> line;
    std::string args;
    const char delim = ' ';
    int status;

    while (1)  {
        std::cout << "$ ";
        line = nrsh_get_line();
//        std::cin >> line;
//        args = nrsh_split_line(line);
//        status = nrsh_run(args);

        line.clear();
        line.shrink_to_fit();
    }
}


int main(int agrc, char **argv) {

    nrsh_loop();

    return EXIT_SUCCESS;
}
