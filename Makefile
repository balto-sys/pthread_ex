jacobi: jacobi.c
	gcc -O0 -Wall -pthread -o jacobiO0 jacobi.c -lm
	gcc -O1 -Wall -pthread -o jacobiO1 jacobi.c -lm
	gcc -O2 -Wall -pthread -o jacobiO2 jacobi.c -lm
	gcc -O3 -Wall -pthread -o jacobiO3 jacobi.c -lm

run:
	/usr/bin/time --format "0 %e %U " ./jacobiO0 >> time 2>&1
	/usr/bin/time --format "1 %e %U " ./jacobiO1 >> time 2>&1
	/usr/bin/time --format "2 %e %U " ./jacobiO2 >> time 2>&1
	/usr/bin/time --format "3 %e %U " ./jacobiO3 >> time 2>&1

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
