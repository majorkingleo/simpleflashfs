/*
 * SimpleIni.h
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */
#pragma once

#include "SimpleFlashFsFileBuffer.h"
#include <static_string.h>
#include <static_format.h>

class SimpleIniBase
{
	SimpleFlashFs::FileBuffer & file;

	const std::array<char,2> comment_signs = {
			{ ',',
			  '\"'
			}
	};

public:
	SimpleIniBase( SimpleFlashFs::FileBuffer & file_ )
	: file ( file_ )
	{}

	SimpleIniBase( const SimpleIniBase & other  ) = delete;

	virtual ~SimpleIniBase() {}


	bool read( const std::string_view & section, const std::string_view & key, std::string_view & value );

	bool read( const std::string_view & section, const std::string_view & key, std::string_view & value, const std::string_view & default_value ) {
		if( !read( section, key, value ) ) {
			value = default_value;
			return true;
		}
		return true;
	}

	bool read( const std::string_view & section, const std::string_view & key, int32_t & value );
	bool read( const std::string_view & section, const std::string_view & key, uint32_t & value );

	bool read( const std::string_view & section, const std::string_view & key, int64_t & value );
	bool read( const std::string_view & section, const std::string_view & key, uint64_t & value );

	bool read( const std::string_view & section, const std::string_view & key, char & value );

	bool read( const std::string_view & section, const std::string_view & key, float & value );
	bool read( const std::string_view & section, const std::string_view & key, double & value );

	bool write( const std::string_view & section,
			    const std::string_view & key,
				const std::string_view & value,
				const std::string_view & comment = {} );

	bool write( const std::string_view & section,
			    const std::string_view & key,
				const uint32_t value,
				const std::string_view & comment = {} );

	bool write( const std::string_view & section,
			    const std::string_view & key,
				const int32_t value,
				const std::string_view & comment = {} );


	bool write( const std::string_view & section,
			    const std::string_view & key,
				const uint64_t value,
				const std::string_view & comment = {} );

	bool write( const std::string_view & section,
				const std::string_view & key,
				const int64_t value,
				const std::string_view & comment = {} );

	bool write( const std::string_view & section,
			    const std::string_view & key,
				const char value,
				const std::string_view & comment = {} );

	bool write( const std::string_view & section,
			    const std::string_view & key,
				const float value,
				const std::string_view & comment = {} );

	bool write( const std::string_view & section,
			    const std::string_view & key,
				const double value,
				const std::string_view & comment = {} );

	bool read_blob( const std::string_view & section,
					const std::string_view & key,
					std::span<std::byte> & blob );

protected:
	std::string_view get_section_name( const std::string_view & line ) const;
	std::tuple<std::string_view,std::string_view> get_key_value( const std::string_view & line ) const;

	virtual std::optional<std::string_view> get_line( SimpleFlashFs::FileBuffer & file ) = 0;

	std::optional<std::size_t> find_section( const std::string_view & section );
	std::optional<std::size_t> find_next_section();
	std::optional<std::size_t> find_next_key();

	bool is_comment( const std::string_view & line ) const {

		if( line.empty() ) {
			return false;
		}

		for( char c : comment_signs ) {
			if( line[0] == c ) {
				return true;
			}
		}

		return false;
	}

	bool append( const std::string_view & section,
				    const std::string_view & key,
					const std::string_view & value,
					const std::string_view & comment = {} );


	bool append_section( const std::string_view & section );

	bool append_key( const std::string_view & key,
			const std::string_view & value,
			const std::string_view & comment = {} );


	bool write( const std::string_view & s );

	bool write( const std::initializer_list<const std::string_view> & il ) {
		for( auto & sv : il ) {
			if( !write( sv )  ) {
				return false;
			}
		}

		return true;
	}

	bool write( const std::span<const std::string_view> & values ) {
		for( auto & sv : values ) {
			if( !write( sv )  ) {
				return false;
			}
		}

		return true;
	}

	bool insert( std::size_t pos_in_file, const std::span<const std::string_view> & values );

	bool insert( std::size_t pos_in_file, const std::string_view & value ) {
		return insert( pos_in_file, std::span<const std::string_view>( {value} ) );
	}

	std::string_view remove_hex_prefix( const std::string_view & s ) const;
};

template<std::size_t N=100>
class SimpleIni : public SimpleIniBase
{
protected:
	Tools::static_string<N> line_buffer;

public:
	SimpleIni( SimpleFlashFs::FileBuffer & file )
	: SimpleIniBase( file )
	{}

	bool write_blob( const std::string_view & section,
			    const std::string_view & key,
				const std::span<const std::byte> & blob,
				const std::string_view & comment = {} )
	{
		// no capacity to read and write this
		if( blob.size() * 2 >= N ) {
			return false;
		}

		Tools::static_string<N> buffer;
		for( unsigned i = 0; i < blob.size(); ++i ) {

			// no space left in buffer
			if( buffer.size() + 2 > buffer.capacity() ) {
				return false;
			}

			buffer += std::string_view(Tools::static_format<10>("%02X", static_cast<const unsigned>(blob[i])));
		}

		// no space left in line
		if( buffer.size() >= line_buffer.capacity() ) {
			return false;
		}

		return SimpleIniBase::write( section, key, buffer, comment );
	}

protected:
	std::optional<std::string_view> get_line( SimpleFlashFs::FileBuffer & file ) override
	{
		auto line = file.get_line<decltype(line_buffer)>();
		if( line ) {
			line_buffer = *line;
			return line_buffer;
		}

		return {};
	}

};


