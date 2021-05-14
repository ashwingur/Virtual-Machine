CC=gcc
CFLAGS=-s -Os -ffunction-sections -fdata-sections -m32

# fill in all your make rules

vm_x2017: vm_x2017.c
	$(CC) $(CFLAGS) $^ -o $@ -lm
	$(CC) $(CFLAGS) -c implementer.c -o implementer -lm


objdump_x2017: objdump_x2017.c
	$(CC) $(CFLAGS) $^ -o $@ -lm
	$(CC) $(CFLAGS) -c implementer.c -o implementer -lm

ascii_to_binary: ascii_to_binary.c
	$(CC) $(CFLAGS) $^ -o $@ -lm
	$(CC) $(CFLAGS) -c implementer.c -o implementer -lm

implementer: implementer.c
	$(CC) $(CFLAGS) -c $^ -o $@ -lm

tests:
	echo "tests"

run_tests: export ASAN_OPTIONS=verify_asan_link_order=0 
run_tests:
	$(CC) $(CFLAGS) ascii_to_binary.c -o ascii_to_binary -lm
	bash test.sh

clean:
	rm vm_x2017
	rm implementer
	rm ascii_to_binary
	rm objdump_x2017

