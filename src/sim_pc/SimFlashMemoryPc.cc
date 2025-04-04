/*
 * SimFlashMemoryInterfacePc.cc
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */
#include <filesystem>
#include <vector>
#include <stderr_exception.h>
#include <format.h>
#include <CpputilsDebug.h>
#include "SimFlashMemoryPc.h"

using namespace Tools;
using namespace SimpleFlashFs::SimPc;

SimFlashFsFlashMemory::SimFlashFsFlashMemory( const std::string & filename_, std::size_t file_size_ )
: filename( filename_ ),
  file(),
  file_size( file_size_ )
{
	if( !std::filesystem::exists( std::filesystem::status(filename)) ) {
		std::ofstream out(filename.c_str());
	}

	std::filesystem::resize_file(filename_, file_size);
	file.open( filename.c_str(), std::ios_base::binary | std::ios_base::in | std::ios_base::out );
}

SimFlashFsFlashMemory::SimFlashFsFlashMemory( const std::string & filename_ )
: filename( filename_ ),
  file(),
  file_size( 0 )
{
	file_size = std::filesystem::file_size(filename);
	file.open( filename.c_str(), std::ios_base::binary | std::ios_base::in | std::ios_base::out );

	if( !file ) {
		throw STDERR_EXCEPTION( Tools::format( "cannot open file '%s'", filename ) );
	}
}


std::size_t SimFlashFsFlashMemory::write( std::size_t address, const std::byte *data, std::size_t size )
{
	file.seekg(address);
	file.write(reinterpret_cast<const char*>(data), size);
	return size;
}

std::size_t SimFlashFsFlashMemory::read( std::size_t address, std::byte *data, std::size_t size )
{
	file.seekg(address);
	file.clear();
	file.read( reinterpret_cast<char*>(data), size );
	long data_read = static_cast<long>(file.tellg()) - static_cast<long>(address);
	return data_read;
}

void SimFlashFsFlashMemory::erase( std::size_t address, std::size_t size )
{
	file.seekg(address);
	std::vector<std::byte> data(size,static_cast<std::byte>(0xFF));
	//data.resize(size,static_cast<std::byte>(0xFF));

	file.write( reinterpret_cast<const char*>(&data[0]), data.size() );

}
