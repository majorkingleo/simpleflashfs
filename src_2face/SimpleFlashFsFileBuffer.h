/*
 * SimpleFlashFsFileBuffer.h
 *
 *  Created on: 02.09.2024
 *      Author: martin.oberzalek
 */

#pragma once

#include "../src/SimpleFlashFsFileInterface.h"
#include <static_vector.h>
#include <span>
#include <optional>

namespace SimpleFlashFs {

class FileBuffer : public FileInterface
{
	class AutoDiscard
	{
		FileBuffer *file_buffer;

	public:
		AutoDiscard( FileBuffer * file_buffer_ )
		: file_buffer( file_buffer_ )
		{}

		~AutoDiscard()
		{
			if( file_buffer ) {
				file_buffer->discard_buffer();
			}
		}

		void release() {
			file_buffer = nullptr;
		}
	};

private:
	SimpleFlashFs::FileInterface & file;
	std::span<std::byte> buffer;
	std::span<std::byte> current_buffer{};
	std::size_t current_buffer_start = 0;
	std::size_t pos = 0;
	bool current_buffer_modified = false;

public:

	FileBuffer( SimpleFlashFs::FileInterface & file_, std::span<std::byte> buf_ )
	: file( file_ ),
	  buffer( buf_ )
	{}

	~FileBuffer() {
		flush();
	}

	bool operator!() const override {
		return file.operator!();
	}

	std::size_t write( const std::byte *data, std::size_t size ) override;

	std::size_t read( std::byte *data, std::size_t size ) override;

	bool flush() override;

	std::size_t tellg() const override {
		if( !current_buffer.empty() ) {
			return current_buffer_start + pos;
		}

		return file.tellg();
	}

	std::size_t file_size() const override {
		return std::max( current_buffer_start + pos, file.file_size() );
	}

	bool eof() const override {
		return file.eof();
	}

	void seek( std::size_t pos_ ) override;

	 bool delete_file() override {
		 return file.delete_file();
	 }

	 bool rename_file( const std::string_view & new_file_name ) override {
		 return file.rename_file( new_file_name );
	 }

	 bool valid() const {
		return !operator!();
	 }

	 std::string_view get_file_name() const override {
		 return file.get_file_name();
	 }

	 bool is_append_mode() const override {
		 return file.is_append_mode();
	 }

	 std::span<std::byte> read( std::size_t size );

	 std::size_t write( const std::span<const std::byte> & data ) {
		 return write( data.data(), data.size() );
	 }

	 friend class AutoDiscard;

private:

	 bool read_to_buffer( std::size_t size );

	 void discard_buffer();
	 bool flush_buffer();
};

template<size_t N>
class StaticFileBuffer : public FileBuffer
{
	Tools::static_vector<std::byte,N> buf{};

public:
	StaticFileBuffer( FileInterface & file_ )
	: FileBuffer ( file_, buf )
	{}


};


} // namespace SimpleFlashFs



