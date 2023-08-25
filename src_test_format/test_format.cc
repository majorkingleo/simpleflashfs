#include <math.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <format.h>
#include <ColoredOutput.h>

using namespace Tools;

static ColoredOutput colored_output;

#define TEST( a, b ) \
 aa = a; b; \
 bb = buffer; \
 std::cout << aa << "\t==\t" << bb << "\t: "; \
 if( aa == bb ) \
	 std::cout << colored_output.color_output( colored_output.GREEN, "true" ); \
 else \
	 std::cout <<  colored_output.color_output( colored_output.RED, "false" ); \
 std::cout << std::endl;


int main( int argc, char **argv )
{
 // minimal_init( argc, argv, "", false, true );

  char buffer[250];
  std::string aa, bb;

  try {
      TEST( format ("%s", "String" ),                 sprintf( buffer, "%s", "String" ));
      TEST( format ("%d", 155 ),                      sprintf( buffer, "%d", 155 ));
      TEST( format ("%f", 155.1 ),                    sprintf( buffer, "%f", 155.1 ) );
      TEST( format ("%f", (double) 155.1 ) ,          sprintf( buffer, "%f", (double) 155.1 ) );
      TEST( format ("%x", 1515 ),                     sprintf( buffer, "%x", 1515 ) );
      TEST( format ("%X", 1515 ),                     sprintf( buffer, "%X", 1515 ) );
      TEST( format ("%05d", 3 ),                      sprintf( buffer, "%05d", 3 ) );
      TEST( format ("%05.f", 3.32 ),                  sprintf( buffer, "%05.f", 3.32 ) );
      TEST( format ("%#5x", 1515 ),                   sprintf( buffer, "%#5x", 1515 ) );
      TEST( format ("%#5X", 1515 ),                   sprintf( buffer, "%#5X", 1515 ) );
      TEST( format ("%0*2$d", 1515, 6 ),              sprintf( buffer, "%0*2$d", 1515, 6 ) );
      TEST( format ("%0*d", 6, 1515 ),                sprintf( buffer, "%0*d", 6, 1515 ) );
      TEST( format ("%*d", 6, 1515 ),                 sprintf( buffer, "%*d", 6, 1515 ) );
      TEST( format ("%2$*1$d", 6, 1515 ),             sprintf( buffer, "%2$*1$d", 6, 1515 ) );
      TEST( format ("%2$d%1$d", 6, 1515 ),            sprintf( buffer, "%2$d%1$d", 6, 1515 ) );
      TEST( format ("%.*f", 6, 32.32 ),               sprintf( buffer, "%.*f", 6, 32.32 ) );
      TEST( format ("%.*2$f", 15.15, 5 ),             sprintf( buffer, "%.*2$f", 15.15, 5 ) );
      TEST( format ("%.*f", 5, 15.15 ),               sprintf( buffer, "%.*f", 5, 15.15 ) );
      TEST( format ("%.3s", "HALLO" ),                sprintf( buffer, "%.3s", "HALLO" ) );
      TEST( format ("pi = %.5f", 4 * atan(1.0)),      sprintf( buffer, "pi = %.5f", 4 * atan(1.0)) );
      TEST( format( "%*d-%*d-%*d", 1, 3, 2, 4, 5, 6), sprintf( buffer, "%*d-%*d-%*d", 1, 3, 2, 4, 5, 6) );
      TEST( format( "[%#8.3x]", 0x42 ),               sprintf( buffer, "[%#8.3x]", 0x42 ) );
      TEST( format( "[%#08x]", 0x42 ),                sprintf( buffer, "[%#08x]", 0x42 ) );
      TEST( format( "[%#8.3o]", 0x42 ),               sprintf( buffer, "[%#8.3o]", 0x42 ) );
      TEST( format( "[%#08o]", 0x42 ),                sprintf( buffer, "[%#08o]", 0x42 ) );
      TEST( format( "[%-#08x]", 0x42 ),               sprintf( buffer, "[%-#08x]", 0x42 ) );
      TEST( format( "[%-#08o]", 0x42 ),               sprintf( buffer, "[%-#08o]", 0x42 ) );

      TEST( format( "%2$s %1$s", "A", "B" ),          sprintf( buffer, "%2$s %1$s", "A", "B" ) );
      TEST( format( "[%]", "A" ),                     sprintf( buffer, "[%]", "A" ) );
      TEST( format( "%", "A" ),                       sprintf( buffer, "%", "A" ) );

      TEST( format( "%03d", (unsigned char)35 ),      sprintf( buffer, "%03d", (unsigned char)35 ) );
      TEST( format( "%03d", (char)35 ),               sprintf( buffer, "%03d", (char)35 ) );
      TEST( format( "%.3d", 35 ),                     sprintf( buffer, "%.3d", 35 ) );
      TEST( format( "%.3d", 12345 ),                  sprintf( buffer, "%.3d", 12345 ) );
      TEST( format( "%c", 35 ),                       sprintf( buffer, "%c", 35 ) );
      TEST( format( "%c", (unsigned int)35 ),         sprintf( buffer, "%c", (unsigned int)35 ) );
      TEST( format( "%c", (long)35 ),                 sprintf( buffer, "%c", (long)35 ) );
      TEST( format( "%c", (unsigned long)35 ),        sprintf( buffer, "%c", (unsigned long)35 ) );
      TEST( format( "%c", (short)35 ),                sprintf( buffer, "%c", (short)35 ) );
      TEST( format( "%c", (unsigned short)35 ),       sprintf( buffer, "%c", (unsigned short)35 ) );

      std::cout << std::endl;


  } catch( std::exception & error ) {
    std::cerr << format( "Error: %s\n", error.what() );
  }

  return 0;
}
