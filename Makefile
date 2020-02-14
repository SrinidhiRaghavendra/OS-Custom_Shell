compile_main: src/mysh.c
	gcc -o build/mysh src/mysh.c

clean:
	rm build/mysh