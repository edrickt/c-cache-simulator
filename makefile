all: csim.c
	gcc -g -std=c99 -Wall -o csim csim.c -lm

ignore: csim.c
	gcc -g -std=c99 -o csim csim.c -lm

run:
	./csim -v -s 2 -E 1 -b 2 -t traces/dave.trace

val:
	valgrind --leak-check=full --track-origins=yes -s ./csim -s 2 -E 2 -b 2 -t traces/trans.trace

clean:
	rm -rf csim

compare: csim csim-ref
	@echo "Compare my program to reference program with different"
	@echo "cache configurations and files. Top is reference program"
	@echo "and bottom is my program."
	@echo ""
	@echo "-s 1 -E 2 -b 1 -t traces/yi2.trace"
	@./csim-ref -s 1 -E 2 -b 1 -t traces/yi2.trace
	@./csim -s 1 -E 2 -b 1 -t traces/yi2.trace
	@echo ""
	@echo "-s 2 -E 3 -b 4 -t traces/dave.trace"
	@./csim-ref -s 2 -E 3 -b 4 -t traces/dave.trace
	@./csim -s 2 -E 3 -b 4 -t traces/dave.trace
	@echo ""
	@echo "-s 2 -E 1 -b 3 -t traces/trans.trace"
	@./csim-ref -s 2 -E 1 -b 3 -t traces/trans.trace
	@./csim -s 2 -E 1 -b 3 -t traces/trans.trace
	@echo ""
	@echo "-s 2 -E 2 -b 3 -t traces/trans.trace"
	@./csim-ref -s 2 -E 2 -b 3 -t traces/trans.trace
	@./csim -s 2 -E 2 -b 3 -t traces/trans.trace
	@echo ""
	@echo "-s 2 -E 4 -b 3 -t traces/trans.trace"
	@./csim-ref -s 2 -E 4 -b 3 -t traces/trans.trace
	@./csim -s 2 -E 4 -b 3 -t traces/trans.trace
	@echo ""
	@echo "-s 5 -E 1 -b 5 -t traces/trans.trace"
	@./csim-ref -s 5 -E 1 -b 5 -t traces/trans.trace
	@./csim -s 5 -E 1 -b 5 -t traces/trans.trace
	@echo ""
	@echo "-s 5 -E 6 -b 5 -t traces/long.trace"
	@./csim-ref -s 5 -E 6 -b 5 -t traces/long.trace
	@./csim -s 5 -E 6 -b 5 -t traces/long.trace
	@echo ""
	@echo "-s 4 -E 2 -b 4 -t traces/long.trace"
	@./csim-ref -s 4 -E 2 -b 4 -t traces/long.trace
	@./csim -s 4 -E 2 -b 4 -t traces/long.trace
	@echo ""
	@echo "-s 10 -E 2 -b 5 -t traces/long.trace"
	@./csim-ref -s 10 -E 2 -b 5 -t traces/long.trace
	@./csim -s 10 -E 2 -b 5 -t traces/long.trace
	@echo ""
	@echo "-s 4 -E 1 -b 4 -t traces/long.trace"
	@./csim-ref -s 4 -E 1 -b 4 -t traces/long.trace
	@./csim -s 4 -E 1 -b 4 -t traces/long.trace
