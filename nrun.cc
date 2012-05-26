//-------------------------------------------------------------------------
// This is the program we used to call NACHOS with different -rs arguments
// when we took the class.  It proves very helpful in seeing if there are
// any circumstances which will result in a fatal error (seg. fault, etc).
// 
// This version simply runs whatever filename is passed from the command
// line with rs values of 1-9999.  Feel free to modify it and use it
// in anyway which will help you to test your project.
//
// Designed by Will Page and Ryan Cunningham for CSCI402 (c) 2003
// University of Southern California
// Updated: 25.Feb.03
//-------------------------------------------------------------------------
// compile: gcc nrun.cc -o nrun
// usage: nrun <nachos testcase>
// ex: nrun ../test/halt 
//
// The last output stream will be saved in "output.nrun"
//-------------------------------------------------------------------------
// Note: there is no sanity checking performed on the command line values.
//       also, -rs xxxx is hard-coded for < 10000 rs iterations, but it is
//	  simple to change the code.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define StartRS		1
#define EndRS		9999
#define WhenToPrint	100

int main(int argc, char **argv)
{
 char call[50] = "nachos -rs xxxx -x ";	// if you need an input stream...
 int error = 0;

 argv++;
 strcat(call, *argv);
 strcat(call, " > output.nrun");	// ... add it here (i.e. < input)

 printf("CSCI402 USC NACHOS RS RUNNER\n");
 printf("aka NRUN: best to run and then go get some coffee.\n");

 printf("\nRunning %s\n", call);

 for(int x = StartRS; x < EndRS && !error; x++)
 {
  call[11] = '0' + x/1000;		// doesn't need % since x < 10000
  call[12] = '0' + (x/100)%10;		// mod 10 pops last digit
  call[13] = '0' + (x/10)%10;
  call[14] = '0' + x%10;
  if(x % WhenToPrint == 0)
	  printf("%d -rs's done\n", x);
  error = system(call);
  if(error)
   printf(" ERROR FOUND! With rs = %d\n", x);
 }
 return error;
}
