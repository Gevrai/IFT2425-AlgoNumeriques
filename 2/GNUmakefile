TP2 = TP2-IFT2425

build: 
	gcc -o ${TP2} ${TP2}.c -lm -lX11 

debug: 
	gcc -g -o ${TP2} ${TP2}.c -lm -lX11 
	gdb --args ${TP2} carfront0.pgm carfront1.pgm 400

rubik: build
	./${TP2} rubikseq0.pgm rubikseq1.pgm 500

salesman: build
	./${TP2} salesman0.pgm salesman1.pgm 200

carfront: build
	./${TP2} carfront0.pgm carfront1.pgm 400
