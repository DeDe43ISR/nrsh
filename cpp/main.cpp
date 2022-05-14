#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <regex>

std::vector<std::string> builtin_str { "cd", "help", "exit", "exec"};

std::vector<char *> split_space (char *to_split, int str_size) {
    std::vector<char *> output(str_size);
    char * token = strtok(to_split, " ");
    for (int counter = 0; token != NULL; counter++) {
        output[counter] = token;
        token = strtok(NULL, " ");
    }
    return output;
}


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
        int len = sizeof(args) / sizeof(args[0]);
        if (len == 1)
            exit (EXIT_SUCCESS);
        else
            exit (atoi(args[1]));
    } else if (!strcmp(args[0], "help")) {
        std::cout << "DeDe Narco Shell\n";
        std::cout << "Type program names and arguments, and hit enter.\n";
        std::cout << "The following are built in:\n";

        for(int i = 0; i < builtin_str.size(); i++)
            std::cout << builtin_str[i] << "\n";
    } else if (!strcmp(args[0], "exec")) {

        args.erase(args.begin());
        execvp(args[0], args.data());
        
    }
    return 0;
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
    int exit_status = 500;
    std::vector<char *> args_str(args.size() + 1);
    //for (std::size_t i = 0; i < args.size(); ++i) {
    //    std::cout << "cmd : " << args[i] << "\n";
    //}
    for (std::size_t i = 0; i < args.size(); ++i) {
        
        //std::cout << "to exec : " << args[i] << "\n";
        //std::cout << "find : " << args[i].find(")") << "\n";

        if (!strcmp(&args[i][0], ";")) {

            //std::cout << "to exec ;; : " << args[i-1] << "\n";
            //std::cout << "find : " << args[i].find(")") << "\n";
            if ( i > 2) {
                if (!(args[i-2].find(")") != std::string::npos) && !(args[i-1].find(")") != std::string::npos)) {
                    temp_exit_status = nrsh_exec(args[i-1]);
                }
            } else {
                temp_exit_status = nrsh_exec(args[i-1]);
            }
        } else if ((!strcmp(&args[i][0], "|") && (!strcmp(&args[i+1][0], "|")))) {

            if (exit_status != 0 && exit_status != 500) {
                temp_exit_status == exit_status;
            } else if (temp_exit_status != 0) {
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

            if (exit_status == 0) {
                temp_exit_status = exit_status;
            } else if (temp_exit_status == 0 || temp_exit_status == 500) {
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
        } else if (!strcmp(&args[i][0], "(")) {
            std::vector<std::string>::iterator it = std::find(args.begin(), args.end(), ")");
            //if (it != args.end())
            //    std::cout << "Error : missing ')'" << "\n";
            int end_index = std::distance(args.begin(), it);
            int status;
            pid_t pid;
            std::vector<std::string> temp_args(end_index-i);
            for (int c = 1; c < (end_index - i); ++c) {
                temp_args[c] = args[c+i];
                //std::cout << temp_args[c] << "\n";
            }
            pid = fork();
            if (pid == 0) {
                temp_exit_status = nrsh_run(temp_args);
                exit(temp_exit_status);
            } else {
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
            temp_exit_status = status;
            exit_status = status;

            i+=end_index;
            //while (!strcmp(&args[i+1][0], " ")) {
            //    std::cout << "cm : " << args[end_index+1] << "\n";
            //    i++;
            //}

        } else if (!strcmp(&args[i][0], "\\")) {
            args.erase(args.begin() + i);
            continue;
        } else if (i == args.size() - 1) {
            
            temp_exit_status = nrsh_exec(args[i]);
        }
    }
}

std::vector<std::string> nrsh_get_line () {

    std::string temp_input;
    std::string input;

    std::getline(std::cin, temp_input);
    if (std::cin.eof()) {
        exit(EXIT_SUCCESS);
    }

    while (temp_input.back() == '\\' || temp_input.back() == '&' || temp_input.back() == '|') {
        while (temp_input.back() == '\\') {
            temp_input.pop_back();
        }
            
        input.append(temp_input);
        std::cout << " $ ";
        std::getline(std::cin, temp_input);
        if (std::cin.eof()) {
            exit(EXIT_SUCCESS);
        }
    }

    input.append(temp_input);
    std::regex re("([();|&]|[^();|&]+)");
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
