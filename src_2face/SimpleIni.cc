/*
 * SimpleIni.cc
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */
#include "SimpleIni.h"
#include <string_utils.h>

using namespace Tools;

bool SimpleIni::read( const std::string_view & section, const std::string_view & key, std::string_view & value )
{
	file.seek(0);
	bool found_section = false;

	for( ;; ) {
		 auto o_line = file.get_line<static_string<100>>();

		 // EOF
		 if( !o_line ) {
			 return false;
		 }

		 auto & line = *o_line;

		 // ignore comments
		 if( line.empty() || line[0] == ';' || line[0] == '#' ) {
			 continue;
		 }

		 // remove white spaces
		 auto sv_line = strip_view( line );

		 // ignore empty lines
		 if( sv_line.empty() ) {
			 continue;
		 }

		 // found a section?
		 if( sv_line[0] == '[' ) {
			 auto current_section = get_section_name( sv_line );
			 if( current_section != section ) {
				 continue;
			 }

			 found_section = true;
			 continue;
		 }
	}

}

std::string_view SimpleIni::get_section_name( const std::string_view & line )
{
	return strip_view( line, "[]" );
}



