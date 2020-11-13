Shell Implementation
====================

Simple linux shell implementation written in C. Assignment for Principles in System Design Course.

Running
-------
```sh
$ make
$ ./bin/shell
```

Features
--------
* Support build-in shell commands: `exit`, `pwd`, `clear` and `cd`
* Output exit status of last terminated process `estatus`
* I/O redirection. Input `<`, output `>`, error `2>`, and both output & error `&>`
* Support multiple background processes, specified with `&`
* Display list of all current background jobs `bglist`
* Bring background job to foreground `fg` or `fg <pid>`
* Allow multiple piping operations
* Signal handling for SIGCHLD, SIGINT, SIGUSR2