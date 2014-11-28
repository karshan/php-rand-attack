OPTIONS:= -l OpenCL

main: main.c
	gcc -Wall -g main.c -o main $(OPTIONS)

clean:
	rm -rf main
