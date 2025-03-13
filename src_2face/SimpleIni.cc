/*
 * SimpleIni.cc
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */
#include "SimpleIni.h"
#include <string_utils.h>
#include <static_vector.h>
#include <static_format.h>
#include <CpputilsDebug.h>
#include <charconv>
#include <string_adapter.h>

#ifndef _WIN32
#    include <alloca.h>
#endif


using namespace Tools;

namespace {
	enum KEY_VALUE {
		KEY = 0,
		VALUE
	};
}

namespace SimpleFlashFs {

static std::array<char,2> _default_comment_signs {
	{
		'\"',
		','
	}
};

const std::span<char> SimpleIniBase::default_comment_signs = _default_comment_signs;

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
			file.seek(pos_before_read_line);
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
		auto cs = get_comment_sign();
		if( !write( { cs, "\t", comment, "\n" } ) ) {
			return false;
		}
	}

	return write( { "\t", key, " = ", value, "\n" } );
}

std::string to_debug_string( std::string s )
{
	s = Tools::substitude( s,  "\n", "\\n" );
	s = Tools::substitude( s,  "\t", "\\t" );
	s = Tools::substitude( s,  std::string(1,'\0'), "\\0" );

	return s;
}

bool SimpleIniBase::insert( std::size_t pos_in_file, const std::span<const std::string_view> & values )
{
	file.seek(pos_in_file);

	std::size_t len = 0;
	for( auto sv : values ) {
		len += sv.size();
	}

	const unsigned BUFFER_SIZE = properties.line_buffer_size;

	char *buffer1 = reinterpret_cast<char*>( alloca( BUFFER_SIZE ) );
	char *buffer2 = reinterpret_cast<char*>( alloca( BUFFER_SIZE ) );
	basic_string_adapter<char> s_buffer_origin1(span_vector(std::span<char>( buffer1, BUFFER_SIZE ) ) );
	basic_string_adapter<char> s_buffer_origin2(span_vector(std::span<char>( buffer2, BUFFER_SIZE ) ) );

	// copy values to insert into buffer1
	{
		for( auto sv : values ) {
			s_buffer_origin1 += sv;
		}
/*
		CPPDEBUG( Tools::format( "want to write : '%s' at pos: %d",
				to_debug_string( std::string( s_buffer_origin1.data(), s_buffer_origin1.size() ) ),
				pos_in_file ) );
*/
	}

	auto p_buffer_a = &s_buffer_origin2;
	auto p_buffer_b = &s_buffer_origin1;

	const std::size_t new_file_size = file.file_size() + len;

	for( std::size_t p = pos_in_file; p < new_file_size && !p_buffer_b->empty();  ) {

		// read data into buffer 2
		std::size_t pos_before_read = file.tellg();

		p_buffer_a->resize( p_buffer_a->capacity() );

		std::span<char> readbuf( p_buffer_a->data(), p_buffer_a->size() );
		file.read( readbuf );
		p_buffer_a->resize(readbuf.size());

/*
		CPPDEBUG( Tools::format( "readed from file: '%s'", to_debug_string( std::string( p_buffer_a->data(), p_buffer_a->size() ) ) ) );

		CPPDEBUG( Tools::format( "wanted to write : '%s'{%d} at: %d file_size: %d",
				to_debug_string( std::string( p_buffer_b->data(), p_buffer_b->size() ) ),
				p_buffer_b->size(),
				pos_before_read,
				file.file_size() ) );
*/

		file.seek(pos_before_read);
		if( file.write( std::span<char>(p_buffer_b->data(), p_buffer_b->size()) ) != p_buffer_b->size() ) {
			return false;
		}
		p += p_buffer_b->size();

		std::swap( p_buffer_a, p_buffer_b );
	}

	return true;
}

bool SimpleIniBase::write( const std::string_view & section,
		const std::string_view & key, const std::string_view & value, const std::string_view & comment )
{
	auto o_section_pos = find_section( section );

	if( !o_section_pos ) {
		return append_section( section ) && append_key( key, value, comment );
	}

	// CPPDEBUG( "section found" );
	std::optional<std::size_t> last_key_end;
	std::size_t section_keys_start = file.tellg();

	for( auto o_next_key_pos = find_next_key(); o_next_key_pos;  o_next_key_pos = find_next_key() ) {

		file.seek(*o_next_key_pos);

		{
			auto o_line = get_line( file );
			if( !o_line ) {
				return false;
			}

			auto key_value = get_key_value( *o_line );
			auto & current_key = std::get<KEY>( key_value );
			auto & current_value = std::get<VALUE>( key_value );

			if( current_key != key ) {
				last_key_end = file.tellg();
				continue;
			}

			if( current_value == value ) {
				// nothing to do
				return true;
			}
		}

		std::size_t start = section_keys_start;
		if( last_key_end ) {
			start = *last_key_end;
		}


		std::size_t end = file.tellg();

		file.seek( start );

		Tools::static_vector<std::string_view,10> sl;
		auto cs = get_comment_sign();

		if( !comment.empty() ) {
			sl.insert( sl.end(), { cs, "\t", comment, "\n" } );
		}

		sl.insert( sl.end(), { "\t", key, " = ", value, "\n" } );

		std::size_t len_to_write = 0;
		for( auto & sv : sl ) {
			len_to_write += sv.size();
		}

		const std::size_t size_to_overwrite = end - start;


		// fill with white spaces
		if(  size_to_overwrite > len_to_write ) {
			if( !write( sl ) ) {
				return false;
			}

			for( unsigned i = 0; i < size_to_overwrite - len_to_write; ++i ) {
				if( !write( " " ) ) {
					return false;
				}
			}

			return true;
		} else {
			char* buffer = reinterpret_cast<char*>(alloca( len_to_write ));

			std::size_t current_pos = 0;
			for( auto & sv : sl ) {
				std::memcpy( buffer + current_pos, sv.data(), sv.size() );
				current_pos += sv.size();
			}

			// 10 - 4 = 6
			if( !write( std::string_view(buffer, size_to_overwrite ) ) ) {
				return false;
			}

			if( !insert( file.tellg(), { std::string_view(buffer + size_to_overwrite, len_to_write - size_to_overwrite ) } ) ) {
				return false;
			}

			return true;
		} // else
	}

	// CPPDEBUG( "key not found" );

	// there may is an extra '\n' before this section
	std::size_t current_pos = file.tellg();
	if( current_pos > 0 ) {
		file.seek(current_pos-1);
		char c;
		if( file.get_char(c) && c == '\n' ) {
			file.seek(current_pos-1);
		}
	}

	Tools::static_vector<std::string_view,10> sl;
	auto cs = get_comment_sign();

	if( !comment.empty() ) {
		sl.insert( sl.end(), { cs, "\t", comment, "\n" } );
	}

	sl.insert( sl.end(), { "\t", key, " = ", value, "\n" } );
/*
	std::size_t x = file.tellg();
	char c;
	file.get_char(c);
	CPPDEBUG( format("current_sign: '%c'", c ) );
	file.seek(x);
*/
	if( !insert( last_key_end ? *last_key_end : file.tellg(), sl ) ) {
		return false;
	}

	return true;
}


bool SimpleIniBase::write(  const std::string_view & section,
							const std::string_view & key,
							const uint32_t value,
							const std::string_view & comment )
{
	auto s = static_format<50>( "%d", value );
	return write( section, key, s, comment );
}

bool SimpleIniBase::write(  const std::string_view & section,
							const std::string_view & key,
							const int32_t value,
							const std::string_view & comment )
{
	auto s = static_format<50>( "%d", value );
	return write( section, key, s, comment );
}

bool SimpleIniBase::write(  const std::string_view & section,
							const std::string_view & key,
							const uint64_t value,
							const std::string_view & comment )
{
	auto s = static_format<50>( "%d", value );
	return write( section, key, s, comment );
}

bool SimpleIniBase::write(  const std::string_view & section,
							const std::string_view & key,
							const int64_t value,
							const std::string_view & comment )
{
	auto s = static_format<50>( "%d", value );
	return write( section, key, s, comment );
}

bool SimpleIniBase::write(  const std::string_view & section,
							const std::string_view & key,
							const char value,
							const std::string_view & comment )
{
	auto s = static_format<50>( "0x%X", static_cast<int>(value) );
	static_string<10> sc;

	switch( value ) {
		case '\n': sc = "\\n"; break;
		case '\t': sc = "\\t"; break;
		case '\r': sc = "\\r"; break;

		default:
			sc += value;
			break;
	}

	auto c = static_format<50>( "%s%s(%s)", comment, comment.empty() ? "" : " ", sc );
	return write( section, key, s, c );
}

bool SimpleIniBase::write(  const std::string_view & section,
							const std::string_view & key,
							const float value,
							const std::string_view & comment )
{
	static_assert( sizeof(float) == sizeof(uint32_t) );

	union f_t {
		float    f;
		uint32_t i;
	};

	f_t f;

	f.f = value;

	auto s = static_format<50>( "0x%X", f.i );
	auto c = static_format<50>( "%s%s(%f)", comment, comment.empty() ? "" : " ", f.f );

	return write( section, key, s, c );
}

bool SimpleIniBase::write(  const std::string_view & section,
							const std::string_view & key,
							const double value,
							const std::string_view & comment )
{
	static_assert( sizeof(double) == sizeof(uint64_t) );

	union f_t {
		double   f;
		uint64_t i;
	};

	f_t f;

	f.f = value;

	auto s = static_format<50>( "0x%X", f.i );
	auto c = static_format<100>( "%s%s(%f)", comment, comment.empty() ? "" : " ", f.f );

	return write( section, key, s, c );
}

namespace {

template<class T>
bool iread( SimpleIniBase & ini, const std::string_view & section, const std::string_view & key, T & value )
{
	std::string_view s_value;

	if( !ini.read( section, key, s_value ) ) {
		return false;
	}

	auto res = std::from_chars( s_value.data(), s_value.data() + s_value.size(), value );

	if( res.ec == std::errc{} ) {
		return true;
	}

	return false;
}

} // namespace

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, int32_t & value )
{
	return iread( *this, section, key, value );
}

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, uint32_t & value )
{
	return iread( *this, section, key, value );
}

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, int64_t & value )
{
	return iread( *this, section, key, value );
}

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, uint64_t & value )
{
	return iread( *this, section, key, value );
}

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, float & value )
{
	std::string_view s_value;

	if( !read( section, key, s_value ) ) {
		return false;
	}

	s_value = remove_hex_prefix( s_value );

	union f_t {
		float    f;
		uint32_t i;
	};

	f_t f;

	auto res = std::from_chars( s_value.data(), s_value.data() + s_value.size(), f.i, 16 );

	if( res.ec == std::errc{} ) {
		value = f.f;
		return true;
	}

	return false;
}

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, double & value )
{
	std::string_view s_value;

	if( !read( section, key, s_value ) ) {
		return false;
	}

	s_value = remove_hex_prefix( s_value );

	union f_t {
		double   f;
		uint64_t i;
	};

	f_t f;

	auto res = std::from_chars( s_value.data(), s_value.data() + s_value.size(), f.i, 16 );

	if( res.ec == std::errc{} ) {
		value = f.f;
		return true;
	}

	return false;
}

std::string_view SimpleIniBase::remove_hex_prefix( const std::string_view & s ) const
{
	if( s.substr(0,2) == "0x" ) {
		return s.substr(2);
	}

	if( s.substr(0,2) == "0X" ) {
		return s.substr(2);
	}

	return s;
}

bool SimpleIniBase::read( const std::string_view & section, const std::string_view & key, char & value )
{
	std::string_view s_value;

	if( !read( section, key, s_value ) ) {
		return false;
	}

	s_value = remove_hex_prefix( s_value );

	int i;
	auto res = std::from_chars( s_value.data(), s_value.data() + s_value.size(), i, 16 );

	if( res.ec == std::errc{} ) {
		value = static_cast<char>(i);
		return true;
	}

	return false;
}

bool SimpleIniBase::read_blob( const std::string_view & section,
							   const std::string_view & key,
							   std::span<std::byte> & blob )
{
	std::string_view s_value;
	if( !SimpleIniBase::read(section, key, s_value) ) {
		return false;
	}

	if( s_value.size() % 2 != 0 ) {
		return false;
	}

	if( s_value.size() / 2 > blob.size() ) {
		return false;
	}

	unsigned i_blob = 0;

	for( unsigned i_buffer = 0; i_buffer < s_value.size(); i_buffer += 2, i_blob++ ) {

		static_string<10> buffer;

		buffer = s_value.substr( i_buffer, 2 );

		unsigned value = 0;
		auto res = std::from_chars( buffer.data(), buffer.data() + buffer.size(), value, 16 );

		if( res.ec != std::errc{} ) {
			return false;
		}

		blob[i_blob] = static_cast<std::byte>(value);
	}

	if( i_blob < blob.size() ) {
		blob = blob.subspan( 0, i_blob );
	}

	return true;
}


} // namespace SimpleFlashFs
