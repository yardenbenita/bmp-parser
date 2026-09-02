CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude

bmp_parser: main.o parser.o bmp.o pixel_data.o logging.o
	$(CC) main.o parser.o bmp.o pixel_data.o logging.o -o bmp_parser
	
main.o: src/main.c include/bmp.h include/pixel_data.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o
	
parser.o: src/parser.c
	$(CC) $(CFLAGS) -c src/parser.c -o parser.o

bmp.o: src/bmp.c include/bmp.h
	$(CC) $(CFLAGS) -c src/bmp.c -o bmp.o
	
pixel_data.o: src/pixel_data.c include/pixel_data.h include/bmp.h
	$(CC) $(CFLAGS) -c src/pixel_data.c -o pixel_data.o

logging.o: src/logging.c
	$(CC) $(CFLAGS) -c src/logging.c -o logging.o

fuzz:
	clang -g -O1 -fsanitize=fuzzer,address,undefined -Iinclude fuzz/fuzz_bmp.c src/parser.c src/bmp.c src/pixel_data.c src/logging.c -o fuzz_bmp

coverage-build:
	clang -g -O0 -fprofile-instr-generate -fcoverage-mapping -fsanitize=fuzzer -Iinclude fuzz/fuzz_bmp.c src/parser.c src/bmp.c src/pixel_data.c src/logging.c -o fuzz_bmp_cov
	
coverage-run: coverage-build
	LLVM_PROFILE_FILE="combined.profraw" ./fuzz_bmp_cov corpus/ -runs=0
	
coverage-data:
	llvm-profdata merge -sparse combined.profraw -o combined.profdata
	
coverage-report:
	llvm-cov report ./fuzz_bmp_cov -instr-profile=combined.profdata
	
coverage-show:
	llvm-cov show ./fuzz_bmp_cov -instr-profile=combined.profdata $(FILE)
	
.PHONY: fuzz coverage-build coverage-run coverage-data coverage-report coverage-show clean
	
clean:
	rm -f main.o parser.o bmp.o pixel_data.o logging.o bmp_parser fuzz_bmp fuzz_bmp_cov combined.profraw combined.profdata
