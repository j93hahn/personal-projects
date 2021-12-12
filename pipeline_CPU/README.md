This project was completed as part of my computer architecture class. For this class, we had to design and construct a CPU within a C simulation using the ARMv8 ISA. The project was broken down into 4 labs. 

- The first lab consisted of developing a single-cycle CPU (sim.c file)
- The second implemented a pipelined 5-cycle CPU design, along with data dependencies and stalling for control path instructions (pipe.c and pipe.h files)
- The third lab involved integrating a branch prediction methodology with the pipeline to better improve processor speed for branch instructions (bp.c and bp.h files)
- The fourth lab involved implementing two level-1 caches into the pipeline for both the memory and instruction set (cache.c and cache.h files)

All the code can be found inside of labs 1-4, with the source code found inside of the src/ sub-directory of each folder. The shell.c and shell.h files were pre-written and serve as the basis for the command line simulation of the CPU. All the other files present were for constructing the Makefile, compiling the test files into assembly language, or for running the actual test scripts on our code. 
