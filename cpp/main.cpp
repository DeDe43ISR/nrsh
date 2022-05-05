#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

//std::vector<std::string> builtin_str = {
//    "cd"
////    "help",
////    "exit"
//};
//int nrsh_cd (std::vector<std::string> args) {
//    if (args.size() == 0) {
//        fprintf(stderr, "nrsh: expected argument to \"cd\"\n");
//    } else {
//        if (chdir(args[1]) != 0) {
//            perror("nrsh: cd error");
//        }
//    }
//    return 0;
//}

int nrsh_run (std::vector<std::string> args) {
    pid_t pid;
    int status, exit_status;

    std::vector<char *> args_str(args.size() + 1);
    for (std::size_t i = 0; i != args.size(); ++i)
    {
        args_str[i] = &args[i][0];
    }

    
    pid = fork();

    if (pid == 0) {
        if (execvp(args_str[0], args_str.data()) == -1) {
            perror("nrsh: fork failed 1");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("nrsh: fork failed 2");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    if (WIFEXITED(status)) {
        exit_status = WEXITSTATUS(status);
        return exit_status;
    }
    return 0;
}

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
//    std::string args;
    const char delim = ' ';
    int status;

    while (1)  {
        std::cout << "$ ";
        line = nrsh_get_line();
//        std::cin >> line;
//        args = nrsh_split_line(line);
        status = nrsh_run(line);

        line.clear();
        line.shrink_to_fit();
    }
}


int main(int agrc, char **argv) {

    nrsh_loop();

    return EXIT_SUCCESS;
}
