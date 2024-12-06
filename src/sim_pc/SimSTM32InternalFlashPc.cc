/*
 * @author Copyright (c) 2024 Martin Oberzalek
 */
#include "SimSTM32InternalFlashPc.h"
#include <cstring>
#include <stderr_exception.h>
#include <static_format.h>

using namespace SimpleFlashFs::SimPc;

SimSTM32InternalFlashPc::SimSTM32InternalFlashPc( const std::string & filename_, std::size_t size_, bool do_mem_mapping_ )
: SimFlashFsFlashMemory( filename_, size_ ),
  do_mem_mapping( do_mem_mapping_ )
{
	init();
}

SimSTM32InternalFlashPc::SimSTM32InternalFlashPc( const std::string & filename_, bool do_mem_mapping_ )
: SimFlashFsFlashMemory( filename_ ),
  do_mem_mapping( do_mem_mapping_ )
{
	init();
}

void SimSTM32InternalFlashPc::init()
{
	mem.resize(file_size);
	mem_written.resize(file_size);
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

	CPPDEBUG( Tools::format( "writing raw page: %d", address / 512 ) );

	for( unsigned i = 0; i < size; i++ ) {
		if( mem_written[address+i] != std::byte(0) ) {
			throw STDERR_EXCEPTION(Tools::format("memory area %d already written", address+i));
		}
	}

	std::memcpy( &mem[address], data, size );
	std::memset( &mem_written[address], 0xFF, size );

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
	std::memset( &mem_written.at(address), 0x00, size );
	return SimFlashFsFlashMemory::erase( address, size );
}
