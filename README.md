# Operating systems

## Implement the following for xv6 operating system to better understand the internals

### Uniq

* The [uniq](src/uniq/uniq.c) command from coreutils that can be called on the shell with or without file redirection
* An extension to the above with -c, -d and -i options enabled

### Shell

A [shell](src/shell/shell.c) that can perform the following

* Command execution
* I/O redirection
* handle pipes appropriately

### Lottery Scheduling

* Implement a lottery scheduling algorithm with weight of the lottery decided by the number of 'tickets' a process has.
* Each process has 20 tickets at start and can be changed using a settickets method, while this is not secure in a production enviroment leading to processes hogging the CPU time, for the purposes of testing, this will do.
* We created a [patch file](src/lottery-scheduling/0001-Implement-lottery-scheduling.patch) that can be run of the repository mentioned in [this](src/lottery-scheduling/README.md) document

### Mutex vs Spin lock, insert times

Implement both [mutex](src/parallel/parallel_mutex.c) locks and [spin](src/parallel/parallel_spin.c) locks to insert 100000 values in a hash table with 5 buckets. We measure times while trying to make sure no values are dropped due to multiple threads acting on the same data structure

* First we implement with just a single mutex lock on the hash table in the insert function and measure times with different amounts of threads
* We then use a spin lock instead of a mutex lock
* Finally, we use multiple mutux lock, with each bucket getting it's own lock as the buckets are mutually exclusive by default.

