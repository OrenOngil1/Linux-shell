This is my implementation of a Linux shell.

Originally developed as a university project — one I particularly enjoyed and felt proud of.
This repo tracks its evolution and cleanup as I revisit and improve the code.

User Instructions:
* Run the shell with ./shell from from the root of the project directory.
* Type "quit" to quit.
* The shell currently supports execution of Linux commands, basic history features, signal handling, piping, and process management.
* For more details, see Supported commands below.

Supported Commands:
* Basic commands:
  * Any executable command
  * quit - quits the shell.
  * I/O redirection — supports < and > like a regular shell.
  * cd - behaves like in the standard Linux shell.
  * procs - prints the processes spawned by the shell in the format:
    <process index> <process PID> <status> <original command>
  * Pipes — for now supports a pipeline between two processes.
* Signal handeling:
  * halt <pid> — sends SIGSTOP to suspend a process.
  * wakeup <pid> — sends SIGCONT to resume a process.
  * ice <pid> — sends SIGINT to terminate a process.
* History commands:
  * hist - prints the last 20 commands by user
  * re-executes the last non-history command (does nothing if none exists).
  * !n — re-executes the n-th most recent non-history command (does nothing if not found).

Push Log:
* Initial Commit:
    Added custom parser (originally, one was provided) and initial shell implementation.
    Parser still needs to be tested.
    Shell will be refactored.
    Uploaded now to avoid manually installing the Unity testing tool.

* Update #1:
    Added unit tests for the parser using Unity.
    Fixed bugs and addressed memory leaks in the parser.
    Began refactoring the shell — further improvements to be evaluated.
    Removed the unnecessary build/ directory from the repository.

* Update #2:
    Added a .gitignore file to exclude build artifacts from version control.
    Re-added the shell executable to the project root — it was previously omitted despite being referenced in the user instructions.