# Functional-Simulator-ARM-Processor
This is a Functional Simulator for ARM Processor. It was developed as a part of Computer Architecture Course CS112 offered at IIIT Delhi. Please see documentation for a detailed explaination of the instruction set supported.

## Authors
- Kushagra Singh - kushagra14056@iiitd.ac.in
- Lohitaksh Parmar - lohitaksh14059@iiitd.ac.in

# Installation
- cd to the root directory of project
- sudo make
- sudo make install

# Test with given programs
- sudo make check

# Execution
- cd to root directory of project
- gcc -o arm ./test/ARMSIM.c -lARMSim    (link with lib ARMSim)
- ./arm ./input.mem    (Provide mem file as argument)
