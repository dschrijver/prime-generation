run: clean main.out
	@./main.out

main.out:
	@clang main.c -o main.out -Wall -Wextra

clean:
	@rm -f main.out
