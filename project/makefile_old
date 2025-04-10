# set this variable to the director in which you saved the common files
commondir = ../common/

all : lab4-1

lab4-1 : lab4-1.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	gcc -Wall -o lab4-1 -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES lab4-1.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

clean :
	rm lab4-1
