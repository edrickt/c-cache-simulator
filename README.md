# cache-simulator

# PROGRAM GENERAL INFORMATION

Language: C
Standard: C99

# COMPILATION AND EXECUTION

To compile: $make or $gcc -g -std=c99 -Wall -o csim csim.c -lm
To run: $./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>

# REFERENCES

No outside code was used. All ideas are my own.

# ISSUES

No known issues, including memory leaks. Valgrind returns
all memory freed, no memory leaks.