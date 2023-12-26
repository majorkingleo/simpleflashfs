/*
 * SimFlashMemoryInterfacePc.cc
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */
#include "SimFlashMemoryInterfacePc.h"
#include <filesystem>
#include <vector>

using namespace SimpleFlashFs::SimPc;

SimFlashFsFlashMemoryInterface::SimFlashFsFlashMemoryInterface( const std::string & filename_, std::size_t file_size_ )
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


std::size_t SimFlashFsFlashMemoryInterface::write( std::size_t address, const std::byte *data, std::size_t size )
{
	file.seekg(address);
	file.write(reinterpret_cast<const char*>(data), size);
	return size;
}

std::size_t SimFlashFsFlashMemoryInterface::read( std::size_t address, std::byte *data, std::size_t size )
{
	file.seekg(address);
	file.read( reinterpret_cast<char*>(data), size );
	long data_read = file.tellg() - static_cast<long>(address);
	return data_read;
}

void SimFlashFsFlashMemoryInterface::erase( std::size_t address, std::size_t size )
{
	file.seekg(address);
	std::vector<std::byte> data(size,static_cast<std::byte>(0xFF));
	//data.resize(size,static_cast<std::byte>(0xFF));

	file.write( reinterpret_cast<const char*>(&data[0]), data.size() );

}
