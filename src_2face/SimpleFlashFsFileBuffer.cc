/*
 * SimpleFlashFsFileBuffer.cc
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */
#include "SimpleFlashFsFileBuffer.h"
#include <CpputilsDebug.h>
#include <static_format.h>
#include <cstring>

#include <string_utils.h>
using namespace Tools;

namespace SimpleFlashFs {

std::size_t FileBuffer::read( std::byte *data, std::size_t size )
{
	if( size > buffer.size() ) {
		if( !discard_buffer() ) {
			return 0;
		}

		return file.read( data, size );
	}

	auto buf = read( size );

	if( buf.empty() ) {
		return 0;
	}

	std::memcpy( data, buf.data(), buf.size() );
	return buf.size();
}

std::span<std::byte> FileBuffer::read( std::size_t size )
{
	if( size > buffer.size() ) {
		CPPDEBUG( "size > buffer size" );
		return {};
	}

	if( !current_buffer.empty() &&
		 pos + size < current_buffer.size() ) {

		 auto ret = current_buffer.subspan( pos, size );
		 pos += size;

		 return ret;
	}

	// wan't to read till the end of file
	if( !current_buffer.empty() &&
		 current_buffer_start + current_buffer.size() >= file_size() ) {

		 std::size_t size_to_read = std::min( size, current_buffer.size() - pos );
		 auto ret = current_buffer.subspan( pos, size_to_read );
		 pos += size_to_read;

		 return ret;
	}


	if( !current_buffer.empty() ) {
		std::size_t pos_in_file = current_buffer_start + pos;

		if( !discard_buffer() ) {
			return {};
		}

		file.seek(pos_in_file);
	}

	if( !read_to_buffer( size ) ) {
		return {};
	}

	return current_buffer.subspan( 0, std::min( size, current_buffer.size() ) );
}

bool FileBuffer::read_to_buffer( std::size_t size )
{
	current_buffer_start = file.tellg();

	std::size_t size_to_read = std::max( size, buffer.size() );
	size_to_read = std::min( file.file_size() - file.tellg(), buffer.size() );

	// EOF reached
	if( size_to_read == 0 ) {
		return false;
	}

	current_buffer = std::span<std::byte>( buffer.data(), size_to_read );

	std::size_t len_read = file.read( current_buffer.data(), size_to_read );

	if( len_read != size_to_read ) {
		CPPDEBUG( Tools::static_format<100>( "failed reading %d data from %s", size_to_read, file.get_file_name() ) );

		if( !discard_buffer() ) {
			return false;
		}

		return false;
	}

	// EOF
	if( size_to_read < size ) {
		pos = size_to_read;
	} else {
		pos = size;
	}

	return true;
}

bool FileBuffer::discard_buffer()
{
	if( !flush_buffer() ) {
		return false;
	}

	pos = 0;
	current_buffer_start = 0;
	current_buffer = {};

	return true;
}

bool FileBuffer::flush_buffer()
{
	if( current_buffer_modified ) {
		CPPDEBUG( static_format<100>( "flushing buffer: current_buffer_start: %d pos: %d size: %d",
				current_buffer_start, pos, current_buffer.size() ) );

		file.seek(current_buffer_start);
		if( file.write(current_buffer.data(),current_buffer.size())  != current_buffer.size() ) {
			return false;
		}
	}

	current_buffer_modified = false;

	return true;
}

bool FileBuffer::flush()
{
	if( !flush_buffer() ) {
		return false;
	}

	return file.flush();
}

bool FileBuffer::seek( std::size_t pos_to_seek_to )
{
	if( !current_buffer.empty() ) {
		if( current_buffer_start <= pos_to_seek_to &&
			pos_to_seek_to < current_buffer_start + current_buffer.size() ) {
			pos = pos_to_seek_to - current_buffer_start;

			// so that tellg is telling the truth
			if( !file.seek( pos_to_seek_to ) ) {
				return false;
			}
			// CPPDEBUG( Tools::format("seeking to pos: %d", pos) );
			return true;
		}
	}

	if( (file.tellg() == pos_to_seek_to) &&
		(current_buffer.empty() || !current_buffer_modified ) ) {
		return true;
	}

	if( !discard_buffer() ) {
		return false;
	}

	if( !file.seek( pos_to_seek_to ) ) {
		return false;
	}

	return true;
}

std::size_t FileBuffer::write( const std::byte *data, std::size_t size )
{
	/*
	std::string s( std::string_view( reinterpret_cast<const char*>(data), size ) );
	s = substitude( s,  "\n", "\\n" );
	s = substitude( s,  std::string(1,'\0'), "\\0" );

	CPPDEBUG( Tools::format( "starting to write '%s'", s ) );
	*/
	//CPPDEBUG( Tools::format( "starting to write '%s'", std::string_view(reinterpret_cast<const char*>(data),size) ) );

	if( size > buffer.size() ) {
		//CPPDEBUG( "size > buffer.size()" );
		auto pos_in_file = file.tellg();
		if( !discard_buffer() ) {
			return 0;
		}
		if( !file.seek( pos_in_file ) ) {
			return 0;
		}

		std::size_t ret = file.write( data, size );

		return ret;
	}

	/**
	 * This algorithm, may rewrites some data to disc, if it's
	 * already in the buffer from reading. But this will write
	 * too much data, when in append mode. So discard the reading buffer.
	 */
	if( !current_buffer.empty() &&
		!current_buffer_modified &&
		file.is_append_mode() ) {
		if( !discard_buffer() ) {
			return 0;
		}
	}

	if( !current_buffer.empty() ) {
		//CPPDEBUG( "!current_buffer.empty() " );

		// enlarge current_buffer
		if( buffer.size() - (pos + 1) > size ) {
			//CPPDEBUG( "enlarging buffer" );
			current_buffer = buffer.subspan( 0, pos + size );
		}

		//CPPDEBUG( Tools::format( "pos: %d size: %d csize: %d",  pos, size,  current_buffer.size() ));

		// overwrite
		if( pos + size <= current_buffer.size() ) {

			//CPPDEBUG( "pos + size < current_buffer.size()" );

			std::memcpy( &current_buffer[pos], data, size );
			current_buffer_modified = true;
			pos += size;

			return size;
		} else {

			auto pos_in_file = current_buffer_start + pos;

			if( !discard_buffer() ) {
				return 0;
			}

			if( !file.seek( pos_in_file ) ) {
				return 0;
			}

			return write( data, size );
		}
	}


	if( current_buffer.empty() ) {

		//CPPDEBUG( "current_buffer.empty()" );

		current_buffer_start = file.tellg();
		pos = 0;

		current_buffer = buffer.subspan( 0, size );
		std::memcpy( current_buffer.data(), data, size );
		current_buffer_modified = true;

		//CPPDEBUG( Tools::format( "pos: %d size: %d current_buffer_start: %d",  pos, size,  current_buffer_start ));

		pos = size;

		return size;
	}

	throw std::out_of_range("should never be reached");
	return 0;
}


} // namespace SimpleFlashFs
