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

		 //CPPDEBUG( format( "line: '%s' found section: %d", *o_line, found_section ));

		 auto & line = *o_line;

		 // ignore comments
		 if( line.empty() || is_comment( line ) ) {
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

			 found_section = ( current_section == section );
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
	return strip_view( strip_view( line, "[]" ) );
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

std::optional<std::size_t> SimpleIniBase::find_section( const std::string_view & section )
{
	file.seek(0);
	bool found_section = false;

	for( ;; ) {
		 auto o_line = get_line(file);

		 // EOF
		 if( !o_line ) {
			 return {};
		 }

		 auto & line = *o_line;

		 // ignore comments
		 if( line.empty() || is_comment( line ) ) {
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

			 if( current_section == section ) {
				return file.tellg();
			 }
		 }
	}

	return {};
}

std::optional<std::size_t> SimpleIniBase::find_next_section()
{
	bool found_section = false;

	for( ;; ) {
		 auto o_line = get_line(file);

		 // EOF
		 if( !o_line ) {
			 return {};
		 }

		 auto & line = *o_line;

		 // ignore comments
		 if( line.empty() || is_comment( line ) ) {
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
				return file.tellg();
		 }
	}

	return {};
}

std::optional<std::size_t> SimpleIniBase::find_next_key()
{
	for( ;; ) {
		 std::size_t pos_before_read_line = file.tellg();
		 auto o_line = get_line(file);

		 // EOF
		 if( !o_line ) {
			 return {};
		 }

		 auto & line = *o_line;

		 // ignore comments
		 if( line.empty() || is_comment( line ) ) {
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
			return {};
		 }

		 if( sv_line.find('=') == std::string::npos ) {
			continue;
		 }

		 return pos_before_read_line;
	}

	return {};
}

bool SimpleIniBase::write( const std::string_view & s )
{
	std::size_t len_written = file.write( s );
	return len_written == s.size();
}

bool SimpleIniBase::append_section( const std::string_view & section )
{
	// append a '\n' to the file, or not
	if( file.file_size() > 0 ) {
		file.seek(file.file_size()-1);
		char c;
		if( file.get_char(c) && c != '\n' ) {
			if( !write("\n") ) {
				return false;
			}
		}
	}

	if( file.tellg() > 0 ) {
		if( !write("\n") ) {
			return false;
		}
	}
/*
	{
		std::size_t pos_to_start = file.tellg();
		char c = '\0';

		for( std::ssize_t pos = pos_to_start - 1; c != '\n' && std::ssize_t >= 0; pos-- ) {
			file.seekg(pos);
			file.get_char(c);

			if( c == ']' ) {

			}
		}
	}
*/
	return write( { "[", section, "]\n" } );
}

bool SimpleIniBase::append_key( const std::string_view & key,
			const std::string_view & value,
			const std::string_view & comment )
{
	if( !comment.empty() ) {
		if( !write( { "# ", comment, "\n" } ) ) {
			return false;
		}
	}

	return write( { "\t", key, " = ", value, "\n" } );
}

bool SimpleIniBase::write( const std::string_view & section,
		const std::string_view & key, const std::string_view & value, const std::string_view & comment )
{
	auto o_section_pos = find_section( section );

	if( !o_section_pos ) {
		return append_section( section ) && append_key( key, value, comment );
	}

	return false;
}


