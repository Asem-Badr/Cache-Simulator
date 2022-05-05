# Cache-Simulator
This is a part of cmu course "introduction to computer systems" which simulates a hardware cache operating.

# discription
This simulator takes a valgrind memory trace as input and starts counting the hits ,misses and evictions.
It uses the least recent used method to determine the victims.

# What I did in this project
I wrote the csim.c file which is the simulator program itself.
the other files are the traces in the traces directory that is used as a test cases and the test and the testing programs 
provided by the course to test the results and the helper files for this testing programs


************************
Running the autograders:
************************

Before running the autograders, compile your code:
    linux> make

Check the correctness of your simulator:
    linux> ./test-csim

******
Files:
******
csim.c       The cache simulator

# Tools for evaluating your simulator 
Makefile     Builds the simulator and tools
README       This file
cachelab.c   Required helper functions
cachelab.h   Required header file
csim-ref*    The executable reference cache simulator
test-csim*   Tests The cache simulator
test-trans.c Tests The transpose function
tracegen.c   Helper program used by test-trans
traces/      Trace files used by test-csim.c
