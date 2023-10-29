# The Branch Predictor Project

## Welcome to the project

This project is based on the 2004 Championship Branch Predictor contest. Each contestant will write a branch predictor that will consume a trace of branches (generated from real execution). Branch predictors are vital in modern processors as they enable efficient instruction execution by anticipating the outcome of conditional branches, reducing processing delays and improving overall CPU performance. In this assignment, we aren't giving you a full fledged processor simulator, but a rather simple model of a branch predictor. This model will have a few functions which you will have to add to, and design 2 new predictors in code. As a skeleton framework, we have provided you with an existing implementation of the GShare Predictor (correlating predictor). 

## The Task
For this task, you will build 2 predictors based on the skeleton code. The first will be a Tournament branch predictor based on the Alpha 21264 processor. The other will be a custom implementation of your own choice which needs to outperform both the GShare and the Tournament predictors. This will also be a competition between the class itself, to get the maximum possible performance from your processor. The hardware budget you are given is 128Kbits + 512 bits (for registers and such), so please make sure that your data structures fit within this budget. 

Here are some papers that you can refer to for the custom design:

### [TAGE](https://www.irisa.fr/caps/people/seznec/JILP-COTTAGE.pdf)
### [Perceptron](https://www.cs.utexas.edu/~lin/papers/hpca01.pdf)
### [YAGS](https://safari.ethz.ch/digitaltechnik/spring2021/lib/exe/fetch.php?media=mudge_yags.pdf)

You can of course search on your own and implement any other branch predictor you want.

Notice you're given all information of the branch/jump: Branch Address, Branch Target, (Taken-Not taken), (Conditional-Unconditional), (Call-Not Call), (Ret-Not Ret), (Direct-NotDirect). You can use all of these to decide how to train your predictor and make prediction (modifying arguments of `train_predictor` and `make_prediction`), but we will only check the accuracy of the prediction on conditional branches.

## Academic Integrity

This assignment is to be done individually by every student. Please make sure you do not copy a single line of code from any source. Not from other students, not from the web, not from anywhere. We have very sophisticated tools to discover if you did. This is a graduate class and we have the very highest expectations for integrity. You should expect that if you do so, even in very small amounts, you will be caught, you will be asked to leave the program, and if an international student, required to leave the country.

## Get Started

Accept the assignment in Github Classroom

https://classroom.github.com/a/zPxlqyq2

Once you have checked out your repository, start adding your code into this. To compile, run the following command from within the `src` directory: `make all` or `make`. This will compile your code and generate output files. You will also get a executable binary called `predictor`. To run this, you need to give the following command:

```
bunzip2 -kc /path/to/trace | ./predictor --predictor_type
```

You will add the tournament code based on the implementation that can be found in the Alpha 21264 paper. There is a slight modification to the paper design - we are using 2 bit saturating counters for the predictor instead of 3.

## Generate New Traces
If you wish to further test your branch predictor, we also provide a branch trajectory generation tool (branchExtractor). If you didn't clone the repo recursively, use the following command to pull this tool.
```shell
git submodule update --init --recursive
```
BranchExtractor uses intel pin tool to inject monitoring code in program. This is how the tool should be called. You can find more details in the README of branchExtractor.
```sh
$ ./branchExtractor/gen_trace.sh <program> <trace_name>
```

## Pull Update
If needed, we also provide a shell script for you to update your repo from the starter repo.
```shell
./pull_update.sh
```
This will back up all the content in ./src into ./src_backup, and reset the whole project (except ./src_backup and its content) to be the same as updated starter repo. Sorry for the potential extra workload brought by this and we will try to avoid using it.

## What should you edit?

You need to edit predictor.c and potentially predictor.h for the most part. Add your functions and make sure they are referenced correctly so that your code runs perfectly. Please do not edit any file other than predictor.c and predictor.h.

## Deliverables

Your github repo contains your implementation and you need to submit it on gradescope. Please try to use the main branch of the git repository for your final submission. We will provide an autograder that gives the performance of your predictor and a leader board shows your ranking.

Along with this, you will also submit a PDF, which will include a brief description of your choice of custom predictor and its implementation. You should also include a table which shows the performance of the tournament predictor as well as your custom predictor for all the given traces. You should also include a table which shows your total hardware budget usage.
