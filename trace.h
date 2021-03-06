/*
traceing functions for debugging c programs.

the snap library is an integral part of the debugging tool box.

	int snap(&location, &what, amount, flag, &string)

	location is an address, this just prints an address of the place 
	where you are calling
	from or maybe the address of the routine calling snap.
	what is the address of the buffer or memmory location to dump.
	amount is the number of bytes to dump.
	flag, 0 is dump in hex, 1 is dump in ascii text, 2 dump in decimal.
	string is a pointer to a string message that heads the snap.

	TRACE("msg");  prints the file name, line number and message.

	TVAL("%<fmt> msg",variable);  this will format and print a variable.


These macros can be left in place in the code and when the -DDEBUG flag is 
not present they will be compiled out of the code stream.


*/

int snapmem(int *, int *, int, int, char *);

#ifdef DEBUG
#define TRACE(msg) printf("%s\n",msg);
#else
#define TRACE(msg)
#endif
