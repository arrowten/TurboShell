#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <functional>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

// Forward declarations
std::string tsh_read_line();
std::vector<std::string> tsh_split_line(const std::string& line);
bool tsh_launch(const std::vector<std::string>& arguments);
size_t tsh_num_builtins();
bool tsh_cd(const std::vector<std::string>& arguments);
bool tsh_help(const std::vector<std::string>& arguments);
bool tsh_exit(const std::vector<std::string>& arguments);
bool tsh_execute(const std::vector<std::string>& arguments);

// Reads a line of input using std::getline
std::string tsh_read_line() {
    std::string line;
    
    if (!std::getline(std::cin, line)) {
        if (std::cin.eof()) {
            std::exit(EXIT_SUCCESS);  // Received EOF
        } 
        else {
            std::perror("readline");
            
            std::exit(EXIT_FAILURE);
        }
    }
    
    return line;
}

// Takes the input and splits it into individual words
std::vector<std::string> tsh_split_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}


bool tsh_launch(const std::vector<std::string>& arguments) {
    if (arguments.empty()) return true;

    std::vector<char*> argv;

    for(const auto& a : arguments) {
        argv.push_back(const_cast<char*>(a.c_str()));
    }

    argv.push_back(nullptr);  // execvp expects a null-terminated array
    pid_t process_id = fork();

    if (process_id == 0) {
        // Child process
        if(execvp(argv[0], argv.data()) == -1) {
            perror("tsh");

            exit(EXIT_FAILURE);
        }
    } 
    else if (process_id < 0) {
        // Error forking
        perror("tsh");
    } 
    else {
        // Parent process
        int status;
        
        do {
            waitpid(process_id, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return true;
}

// Built-in command function signature
using builtin_function = std::function<bool(const std::vector<std::string>&)>;

// Map of built-in command names to their functions
std::unordered_map<std::string, builtin_function> builtin_commands = {
    {"cd", tsh_cd},
    {"help", tsh_help},
    {"exit", tsh_exit}
};

// Return number of built-in commands
size_t tsh_num_builtins() {
    return builtin_commands.size();
}

// Implementation of 'cd' command
bool tsh_cd(const std::vector<std::string>& arguments) {
    if (arguments.size() < 2) {
        std::cerr << "tsh: expected argument to \"cd\"\n";
    } 
    else {
        if (chdir(arguments[1].c_str()) != 0) {
            std::cerr << "tsh: " << strerror(errno) << "\n";
        }
    }

    return true;
}

// Implementation of 'help' command
bool tsh_help(const std::vector<std::string>& arguments) {
    std::cout << "TurboShell (TSH) — a minimal custom shell\n";
    std::cout << "Built-in commands:\n";
    
    for (const auto& [name, _] : builtin_commands) {
        std::cout << "  " << name << "\n";
    }
    
    return true;
}

// Implementation of 'exit' command
bool tsh_exit(const std::vector<std::string>& args) {
    return false; // signals shell to exit
}

// Interpretation and execution of command
bool tsh_execute(const std::vector<std::string>& arguments) {
    if (arguments.empty()) {
        // An empty command was entered.
        return true;
    }

    // Check if the command is a built-in
    auto it = builtin_commands.find(arguments[0]);
    
    if (it != builtin_commands.end()) {
        return it->second(arguments);  // call the corresponding function
    }

    tsh_launch(arguments);  // Otherwise, launch external command

    return true;
}

// Runs continously until the user decides to exit
void tsh_loop() {
    std::string line;
    std::vector<std::string> arguments;
    bool status = true;

    do {
        // Prompt to the terminal
        std::cout << "--> ";
        // Reads a full line of input from the user
        line = tsh_read_line();
        // Splits the line into words
        arguments = tsh_split_line(line);
        // Passes the parsed command (arguments) to an execution function
        status = tsh_execute(arguments);
    } while (status);
}

int main(int argc, char* argv[]) {
    // Convert arguments to a vector of strings for modern usage
    std::vector<std::string> arguments(argv, argv + argc);

    // Run command loop
    tsh_loop();

    return EXIT_SUCCESS;
}