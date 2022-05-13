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

std::vector<char *> split_space (char *to_split, int str_size) {
    std::vector<char *> output(str_size);
    char * token = strtok(to_split, " ");
    for (int counter = 0; token != NULL; counter++) {
        output[counter] = token;
        token = strtok(NULL, " ");
    }
    return output;
}

int nrsh_exec(std::string arg) {

    int str_size = arg.size() + 1;
    std::vector<char *> args_str(str_size);

    args_str = split_space(&arg[0], str_size);

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
        //std::cout << "status : " << exit_status << "\n";
        return exit_status;
    }
    return 0;
}

int nrsh_run (std::vector<std::string> args) {

    int temp_exit_status = 500;
    std::vector<char *> args_str(args.size() + 1);
    for (std::size_t i = 0; i < args.size(); ++i) {
        
        //std::cout << "to exec : " << args[i] << "\n";

        if (!strcmp(&args[i][0], ";")) {

            temp_exit_status = nrsh_exec(args[i-1]);

        } else if ((!strcmp(&args[i][0], "|") && (!strcmp(&args[i+1][0], "|")))) {

            if (temp_exit_status != 0) {
                //std::cout << "exec or : " << args[i-1] << "\n";
                temp_exit_status = nrsh_exec(args[i-1]);
            } else {
                i+=2;
                continue;
            }
            if (temp_exit_status == 0) {
                i+=2;
                continue;
            } else {
                temp_exit_status = nrsh_exec(args[i+2]);
                i+=2;
                continue;
            }

        } else if ((!strcmp(&args[i][0], "&") && (!strcmp(&args[i+1][0], "&")))) {

            if (temp_exit_status == 0 || temp_exit_status == 500) {
                //std::cout << "exec and : " << args[i-1] << "\n";
                temp_exit_status = nrsh_exec(args[i-1]);
            } else {
                i+=2;
                continue;
            }
            if (temp_exit_status == 0) {
                //std::cout << "exec 2 and : " << args[i+2] << "\n";
                temp_exit_status = nrsh_exec(args[i+2]);
                i+=2;
                continue;
            } else {
                i+=3;
                continue;
            }
        } else if (i == args.size() - 1) {
            
            temp_exit_status = nrsh_exec(args[i]);
        }
    }
}

std::vector<std::string> nrsh_get_line () {

    std::string input;

    std::getline(std::cin, input);
    if (std::cin.eof()) {
        exit(EXIT_SUCCESS);
    }
    std::regex re("([;|&]|[^;|&]+)");
    std::sregex_token_iterator first{input.begin(), input.end(), re}, last;
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
