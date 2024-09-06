/*
 * SimpleIni.h
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */
#pragma once

#include "SimpleFlashFsFileBuffer.h"
#include <static_string.h>

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

	bool write( const std::string_view & section,
			    const std::string_view & key,
				const std::string_view & value,
				const std::string_view & comment = {} );

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

	bool insert( std::size_t pos_in_file, const std::span<const std::string_view> & values );
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


