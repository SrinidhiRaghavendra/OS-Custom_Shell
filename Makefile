compile_main: src/mysh.c src/execute.c src/execute.h
	gcc src/mysh.c src/execute.c -o build/mysh 

clean:
	rm build/mysh