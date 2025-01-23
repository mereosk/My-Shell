# **myshell Project**

## **Project Structure**
My project consists of three directories: `include`, `modules`, and `program`.

- The `program` folder contains the main program file, `mysh.c`, which includes the `main` function.
- The `modules` folder includes:
  - **`parsing.c`:** Handles the parsing of commands for my shell.
  - **`bash_interface.c`:** Contains functions for creating the shell's interface, which are invoked from `parsing.c`. This separation was made to keep `parsing.c` more concise.
  - **`ADTList.c`, `ADTMap.c`, and `ADTVector.c`:** Implementations of a list, a hash map, and a dynamic array, respectively.
    - These data structures are inspired by the implementations from Prof. Chatzikokolakis' 2020 materials. Although I have made several modifications, I decided not to rewrite the structures entirely, as this would be similar to using STL in C++. This approach was approved during the course, as long as only the structures were reused.

- The `include` folder contains the corresponding `.h` files with function declarations for the aforementioned files.
- Additionally, there is a `Makefile` (located outside the directories), which compiles the program using the `make` command.

---

## **Execution Steps**
1. Navigate to the project root directory (where the `include`, `modules`, and `program` folders are located).
2. Run `make` to compile the program.
3. Execute the shell by running `./program/mysh`.

### **Additional Makefile Commands**
- **`make run`:** Compiles and directly runs the program.
- **`make valgrind`:** Runs the program with the Valgrind debugger.  
  > Note: Due to a Makefile-specific quirk, the `SIGTSTP` signal is not blocked. Therefore, I do not recommend using this command.

---

## **Parsing**
The parsing logic is implemented using the **state machine technique**, where each character in the command changes the state. Commands are first split into subcommands using `strtok` whenever the semicolon (`;`) is encountered. Below are the states:

1. **EXPCOMMAND (expect command):** When waiting for a command (e.g., `ls`, `sort`, `./signal`) at the beginning or after a pipe (`|`).
2. **EXPARGUMENT (expect argument):** When waiting for an argument after a command.
3. **OUTREDIRECT:** Handles output redirection (`>`). The shell supports multiple redirections like `ls > output1 > output2`, where data is redirected to the last file (like in bash). If `>>` is encountered, the `appendFlag` is set to `true`, enabling appending to the file during execution.
4. **INREDIRECT:** Handles input redirection (`<`) with similar behavior as output redirection. However, the input files must exist.

When encountering a null character (`'\0'`) or `&`, the program executes the `execute_command` function based on the current state. This function, along with others, is thoroughly documented in the code.

---

## **Execute Command Function**
The `execute_command` function takes the following as input:
- The list of parsed commands.
- The list of arguments for each command.
- Input and output redirection files (if any).
- Two flags:
  - **`appendFlag`:** Indicates whether to append data to the file.
  - **`backgroundFlag`:** Indicates whether the command should execute in the background.

The function processes pipes, forks child processes, and sets up appropriate file descriptors using `dup2` and `close`. It prepares arguments for each command and calls `execvp` for execution. A loop ensures all child processes are awaited using `waitpid`.

---

## **Background Execution**
When the `backgroundFlag` is `true`, commands execute in the background. This introduces one key difference:  
- Child processes are placed in separate process groups.  

This approach provides several benefits:
1. Signals like `Ctrl+C` and `Ctrl+Z` do not affect background processes.
2. Background processes can be managed via `waitpid`.
3. Prevents input conflicts between background and foreground processes by leveraging process groups.

Additionally, before parsing, a `while` loop using `waitpid` with the `WNOHANG` option ensures completed background processes are handled, preventing zombie processes.

---

## **Features**
### **History**
- A dynamic vector stores the command history. Unlike the exercise requirements (limiting history to 20 commands), this implementation dynamically doubles the vector’s size when full.
- Duplicate commands (consecutive repeats) are not added to the history.
- **History commands supported:**
  - `!n`: Executes the nth command in history.
  - `!-n`: Executes the command n steps before the last one.
  - `!!`: Executes the most recent command.
  - `!string`: Searches for the most recent command starting with `string`.

All commands support argument addition (e.g., `!2 modules` executes `ls modules` if the 2nd command was `ls`).

---

### **Aliases**
Aliases are implemented using a hash map, where the key is the alias name, and the value is the actual command.
- Example: `createalias myhome "/home/some/thing"`.
- The `djb2` hash function was chosen for its simplicity and performance.

---

### **Wildcards**
Wildcard handling is implemented using `glob`, which processes any wildcards in the command arguments before execution.

---

### **Change Directory & Environment Variables**
- **Change Directory:** Fully implemented using the `chdir` function.
- **Environment Variables:** Before parsing, all environment variables in the command are replaced using the `replace_enc_vars` function.

---

