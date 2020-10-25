FLAGS = -O1 -Wall -fsanitize=address,undefined

n: source/n/n.c 
	gcc -g $(FLAGS) -I /usr/local/Cellar/llvm/10.0.1_1/include -L /usr/local/Cellar/llvm/10.0.1_1/lib -o n source/n/n.c -lclang

# makefile is a work in progress! not finished.