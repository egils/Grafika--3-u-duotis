#*****************************************************************************
#*  Kurso "Transliavimo metodai" 2012/13 m.m. rudens (7) sem.             
#*  3-as praktinis darbas. Variantas Nr. 324                               
#*  Darbà atliko: Egidijus Lukauskas                                          
#*****************************************************************************

CC = g++

all:	compile run

compile:	main.cpp
	$(CC) -o vertice main.cpp lib/Matrices.cpp lib/arcball.cpp -lglut -lGLU -lGL
	
run:
	./vertice