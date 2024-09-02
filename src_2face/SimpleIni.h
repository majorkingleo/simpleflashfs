/*
 * SimpleIni.h
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */
#pragma once

#include "../src/SimpleFlashFsFileInterface.h"

class SimpleIni
{
	SimpleFlashFs::FileInterface & file;

public:
	SimpleIni( SimpleFlashFs::FileInterface & file_ )
	: file ( file_ )
	{}

	SimpleIni( const SimpleIni & other  ) = delete;


	bool read( const std::string_view & section, const std::string_view & key, std::string_view & value );

	bool read( const std::string_view & section, const std::string_view & key, std::string_view & value, const std::string_view & default_value ) {
		if( !read( section, key, value ) ) {
			value = default_value;
			return true;
		}
		return true;
	}

	bool write( const std::string_view & section, const std::string_view & key, const std::string_view & value );

private:

};

