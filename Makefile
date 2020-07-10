jacobi: jacobi.c
	gcc -O0 -Wall -pthread -o jacobiO0 jacobi.c -lm
	gcc -O1 -Wall -pthread -o jacobiO1 jacobi.c -lm
	gcc -O2 -Wall -pthread -o jacobiO2 jacobi.c -lm
	gcc -O3 -Wall -pthread -o jacobiO3 jacobi.c -lm

run:
	/usr/bin/time ./jacobiO0
	/usr/bin/time ./jacobiO1
	/usr/bin/time ./jacobiO2
	/usr/bin/time ./jacobiO3

clean:
	rm -f ./jacobiO?

ex1: ex1.c
	gcc -Wall -pthread -o ex1 ex1.c

ex2: ex2.c
	gcc -Wall -pthread -o ex2 ex2.c

ex3: ex3.c
	gcc -Wall -pthread -o ex3 ex3.c

ex4: ex4.c
	gcc -Wall -pthread -o ex4 ex4.c
