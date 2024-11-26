/*
 * @author Copyright (c) 2024 Martin Oberzalek
 */
#include "SimSTM32InternalFlashPc.h"
#include <cstring>
#include <stderr_exception.h>
#include <static_format.h>

using namespace SimpleFlashFs::SimPc;

SimSTM32InternalFlashPc::SimSTM32InternalFlashPc( const std::string & filename_, std::size_t size_ )
: SimFlashFsFlashMemory( filename_, size_ )
{
	init();
}

SimSTM32InternalFlashPc::SimSTM32InternalFlashPc( const std::string & filename_)
: SimFlashFsFlashMemory( filename_ )
{
	init();
}

void SimSTM32InternalFlashPc::init()
{
	mem.resize(file_size);
	file.read(reinterpret_cast<char*>(mem.data()), file_size);
}

std::size_t SimSTM32InternalFlashPc::write( std::size_t address, const std::byte *data, std::size_t size )
{
	if( address >= mem.size() ) {
		throw STDERR_EXCEPTION( Tools::static_format<100>("address %d is out of range. max: %d", address, mem.size() ) );
	}

	if( address + size > mem.size() ) {
		throw STDERR_EXCEPTION( Tools::static_format<100>( "address + size %d + %d = %d >= %d is out of range", 
															address, size, address + size, mem.size() ) );
	}

	std::memcpy( &mem[address], data, size );
	return SimFlashFsFlashMemory::write( address, data, size );
}

std::size_t SimSTM32InternalFlashPc::read( std::size_t address, std::byte *data, std::size_t size )
{
	std::memcpy( data, &mem.at(address), size );
	return size;
}

void SimSTM32InternalFlashPc::erase( std::size_t address, std::size_t size )
{
	std::memset( &mem.at(address), 0xFF, size );
	return SimFlashFsFlashMemory::erase( address, size );
}
