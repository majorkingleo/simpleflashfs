/*
 * SimpleFlashFsFlashMemoryInterface.h
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */

#pragma once
#include <cstddef>
#include <string>
#include <fstream>

namespace SimpleFlashFs {

class FileInterface
{
public:
	virtual ~FileInterface() {}

	virtual bool operator!() const = 0;

	virtual std::size_t write( const std::byte *data, std::size_t size ) = 0;

	virtual std::size_t read( std::byte *data, std::size_t size ) = 0;

	virtual bool flush() = 0;

	virtual std::size_t tellg() const = 0;

	virtual std::size_t file_size() const = 0;

	virtual bool eof() const = 0;

	virtual bool seek( std::size_t pos_ ) = 0;

	virtual bool delete_file() = 0;

	virtual bool rename_file( const std::string_view & new_file_name ) = 0;

	// comfort functions

	bool valid() const {
		return !operator!();
	}

	virtual std::string_view get_file_name() const = 0;

	virtual bool is_append_mode() const = 0;
};


} // namespace SimpleFlashFs

