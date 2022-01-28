mat: mat.c
	cc -g -Wall mat.c -o mat

test: mat
	./mat < test_script | diff - test_out
