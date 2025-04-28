# set this variable to the directory in which you saved the common files
commondir = ../common/

all : lab4-1 multiple-four lab4-3 lab4-4 lab4-5 lab4-6 project

project : project.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o project -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES project.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lX11 -lGL -lm

multiple-four : multiple-four.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o multiple-four -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES multiple-four.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lX11 -lGL -lm

lab4-3 : lab4-3.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o lab4-3 -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES lab4-3.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lX11 -lGL -lm

lab4-4 : lab4-4.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o lab4-4 -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES lab4-4.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lX11 -lGL -lm

lab4-5 : lab4-5.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o lab4-5 -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES lab4-5.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lX11 -lGL -lm

lab4-6 : lab4-6.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	g++ -Wall -o lab4-6 -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES lab4-6.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lX11 -lGL -lm



clean :
	rm project
	rm multiple-four
	rm lab4-3
	rm lab4-4
	rm lab4-5
	rm lab4-6

