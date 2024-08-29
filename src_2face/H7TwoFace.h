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

#include "../src/SimpleFlashFsFileInterface.h"
#include "../src/SimpleFlashFsFlashMemoryInterface.h"

class H7TwoFace
{
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

	static void set_memory_interface( SimpleFlashFs::FlashMemoryInterface *mem1, SimpleFlashFs::FlashMemoryInterface *mem2 );
};

