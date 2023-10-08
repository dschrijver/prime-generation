run: clean main.out
	@./main.out

main.out:
	@clang main.c -o main.out

clean:
	@rm -f main.out

