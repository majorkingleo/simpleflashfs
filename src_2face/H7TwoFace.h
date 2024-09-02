/*
 * H7TwoFace.h
 *
 *  Created on: 29.08.2024
 *      Author: martin.oberzalek
 */

#pragma once

#include <span>
#include <string>
#include <memory>
#include <functional>

#include "../src/SimpleFlashFsFileInterface.h"
#include "../src/SimpleFlashFsFlashMemoryInterface.h"

class H7TwoFace
{
public:
	struct Stat
	{
		std::size_t largest_file_size = 0; // in bytes
		std::size_t trash_size = 0; // in bytes
		std::size_t used_inodes = 0; // number of inodes in use
		std::size_t trash_inodes = 0; // number of inodes to delete
		std::size_t free_inodes = 0;
		std::size_t max_number_of_files = 0;
		std::size_t number_of_files = 0;
		std::size_t max_file_size = 0;
		std::size_t max_path_len = 0;
		std::size_t free_space = 0;
	};

protected:
	class Destroyer
	{
	public:
		void operator()( SimpleFlashFs::FileInterface* fs ) const {
			fs->~FileInterface();
		}
	};

public:
	using file_handle_t = std::unique_ptr<SimpleFlashFs::FileInterface,Destroyer>;

public:
	static file_handle_t open( const std::string_view & name, std::ios_base::openmode mode );
	static std::span<std::string_view> list_files();
	static Stat get_stat();

	static void set_memory_interface( SimpleFlashFs::FlashMemoryInterface *mem1, SimpleFlashFs::FlashMemoryInterface *mem2 );
	static void set_crc32_func( std::function<uint32_t(const std::byte* data, size_t len)> fs_crc32_func );
};

