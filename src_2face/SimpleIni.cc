/*
 * SimpleIni.cc
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */
#include "SimpleIni.h"
#include <string_utils.h>

using namespace Tools;

namespace {
	enum KEY_VALUE {
		KEY = 0,
		VALUE
	};
}

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, std::string_view & value )
{
	file.seek(0);
	bool found_section = false;

	for( ;; ) {
		 auto o_line = get_line(file);

		 // EOF
		 if( !o_line ) {
			 return false;
		 }

		 CPPDEBUG( format( "line: '%s' found section: %d", *o_line, found_section ));

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

		 if( !found_section ) {
			 continue;
		 }

		 if( sv_line.find( '=' ) == std::string::npos ) {
			 continue;
		 }

		 auto key_value = get_key_value( sv_line );

		 auto & current_key   = std::get<KEY>( key_value );
		 auto & current_value = std::get<VALUE>( key_value );

		 if( current_key != key ) {
			 continue;
		 }

		 value = current_value;
		 return true;
	}

	return false;
}

std::string_view SimpleIniBase::get_section_name( const std::string_view & line ) const
{
	return strip_view( line, "[]" );
}

std::tuple<std::string_view,std::string_view> SimpleIniBase::get_key_value( const std::string_view & line ) const
{
	std::tuple<std::string_view,std::string_view> key_value{};

	std::string::size_type pos = line.find( '=' );

	// should not happen
	if( pos == std::string::npos ) {
		return {};
	}

	std::get<KEY>(key_value) = strip_view( line.substr( 0, pos ) );
	std::get<VALUE>(key_value) = strip_view( line.substr( pos + 1 ) );

	return key_value;
}

