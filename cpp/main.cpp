#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <regex>

std::vector<std::string> builtin_str { "cd", "help", "exit"};

int nrsh_builtin (std::vector<char *> args) {
    //strcmp return value if strings no equal
    if (!strcmp(args[0], "cd")) {
        if (args[1] == NULL) {
            std::cerr << "nrsh: expected argument to \"cd\"\n";
        } else { 
            if (chdir(args[1]) != 0) {
                std::cerr << "nrsh: cd error";
            }
        }
        return 0;
    } else if (!strcmp(args[0], "exit")) {
        exit (EXIT_SUCCESS);
    } else if (!strcmp(args[0], "help")) {
        std::cout << "DeDe Narco Shell\n";
        std::cout << "Type program names and arguments, and hit enter.\n";
        std::cout << "The following are built in:\n";

        for(int i = 0; i < builtin_str.size(); i++)
            std::cout << builtin_str[i] << "\n";
    }
    return 0;
}

int nrsh_exec (std::vector<char *> args_str) {

    if (std::find(builtin_str.begin(), builtin_str.end(), args_str[0]) != builtin_str.end()) {
        return (nrsh_builtin(args_str));
    }
    pid_t pid;
    int status, exit_status, negate = 0;

    if (!strcmp(args_str[0] ,"!")) {
        std::cout << " in";
        negate = 1;
        args_str.erase(args_str.begin());
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
        if (negate == 1) {
            if (exit_status == 0) {
                exit_status = 1;
            } else {
                exit_status = 0;
            }
        }
        std::cout << "status : " << exit_status << "\n";
        return exit_status;
    }
    return 0;
}
std::vector<char *> split_space (char *to_split, int str_size) {
    std::vector<char *> output(str_size);
    char * token = strtok(to_split, " ");
    for (int counter = 0; token != NULL; counter++) {
        output[counter] = token;
        token = strtok(NULL, " ");
    }
    return output;
}

int nrsh_run (std::vector<std::string> args) {

    int exit_status = 0, str_c = 0;
    std::vector<char *> args_str(args.size() + 1);
    //std::cout << "\ncmd : " << args[0] << "\n";
    //std::cout << "\nsize : " << args.size() << "\n";
    for (std::size_t i = 0; i != args.size(); ++i) {


        if (!strcmp(&args[i][0], ";")) {
            int str_size = args[i-1].size() + 1;
            std::vector<char *> temp_args_str(str_size);

            temp_args_str = split_space (&args[i-1][0], str_size);

            exit_status = nrsh_exec (temp_args_str);
        }
    //        args_str.erase(args_str.begin(), args_str.begin() + str_c);
    //        str_c = 0;
    //    } else {
    //    args_str[str_c] = &args[vec_c][0];
    //    str_c++;
    //    }
    //}
    //std::cout << "\ncmd2 : " << args_str[1] << "\n";
    //for(int i = 0; i < args.size(); i++)
    //    std::cout << args[i] << "---";
    //return nrsh_exec (args_str);
    }

}


std::vector<std::string> nrsh_get_line () {

    std::string input;

    std::getline(std::cin, input); // use getline() function to read a line of string and store into S variable.
    //if (std::cin.eof()) {
    //    exit(EXIT_SUCCESS);
    //}

    //std::stringstream X(S); // X is an object of stringstream that references the S string

    //// use while loop to check the getline() function condition
    //while (std::getline(X, T, ' ')) {
    //    /* X represents to read the string from stringstream, T use for store the token string and,
    //     ' ' whitespace represents to split the string where whitespace is found. */
    //    output.push_back(T);
    //    //cout << T << endl; // print split string
    //}
    std::regex re("([;]|[^;]+)");
    std::sregex_token_iterator first{input.begin(), input.end(), re}, last;//the '-1' is what makes the regex split (-1 := what was not matched)
    std::vector<std::string> output{first, last};

    return output;
}
void nrsh_loop(void) {

    std::vector<std::string> line;
    const char delim = ' ';
    int status = 0;


    while (1) {
        char cpwd[FILENAME_MAX];
        getcwd (cpwd, FILENAME_MAX);
        std::cout << cpwd << " $ ";
        line = nrsh_get_line();

        if (line.empty())
            continue;

        status = nrsh_run(line);

        line.clear();
        line.shrink_to_fit();
    }
}


int main(int agrc, char **argv) {

    nrsh_loop();

    return EXIT_SUCCESS;
}
