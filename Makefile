build_name = main
test_name = test
bench_name = bench
graph_name = graph_bench

default:
	@echo "This is Make for C++ Chess."
	@echo "Build profiles supported are:"
	@echo "make test - compile and run tests on all components."
	@echo "make bench - compile and run a benchmark with text readout."
	@echo "make graph - compile and run a benchmark and generate a callgraph."

test:
	@echo "scan_tests:"
	@g++ -std=c++2a -O1 -Wall -Wextra -Werror -Wpedantic -fmax-errors=5 SquareIteratorTests.cpp -o $(test_name)
	./$(test_name)
	#@echo "movegen_tests:"
	#@g++ -std=c++2a -O1 -Wall -Wextra -Werror -Wpedantic MoveGenTests.cpp -o $(test_name)
	#./$(test_name)

bench:
	g++ -std=c++2a -Ofast -Wall -Wextra -Werror -Wpedantic bench.cpp -o $(bench_name)
	./$(bench_name)

graph_bench:
	g++ -std=c++2a -pg -Wall -Wextra -Werror -Wpedantic bench.cpp -o $(graph_name)
	./$(graph_name)
	gprof ./$(graph_name) | gprof2dot -s | dot -Tpng -o graph_bench.png

clean:
	rm -f $(build_name)
	rm -f $(test_name)
	rm -f $(bench_name)
	rm -f $(graph_name)
	rm -f gmon.out
	rm -f graph_bench.png
