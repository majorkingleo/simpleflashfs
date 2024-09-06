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

	bool write( const std::string_view & section, const std::string_view & key, const std::string_view & value );

protected:
	std::string_view get_section_name( const std::string_view & line ) const;
	std::tuple<std::string_view,std::string_view> get_key_value( const std::string_view & line ) const;

	virtual std::optional<std::string_view> get_line( SimpleFlashFs::FileBuffer & file ) = 0;
};

template<std::size_t N=100>
class SimpleIni : public SimpleIniBase
{
protected:
	char acbuf1[100];
	Tools::static_string<N> line_buffer;
	char acbuf2[100];

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


