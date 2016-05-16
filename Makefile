#This is a small project for my operating system class. 2014

rosh.x:	rosh.c test.x 
	gcc rosh.c -o rosh.x
		
test.x:	test.c
	gcc test.c -o test.x
	
clean:	
	rm -f *.out *.x *~
