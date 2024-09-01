/**
 * class for simulating the internal flash from
 * STM32 microprocessor series.
 *
 * For writing you have to use write functions,
 * but for reading: the memory is mapped to the
 * address space. So instead of copying the memory
 * to somewhere via memcpy() it would be possible
 * just to point to the target address.
 *
 * @author Copyright (c) 2024 Martin Oberzalek
 */
#pragma once

#include "../SimpleFlashFsFlashMemoryInterface.h"
#include "SimFlashMemoryPc.h"
#include <vector>

#include <string>
#include <fstream>

namespace SimpleFlashFs {
namespace SimPc {

class SimSTM32InternalFlashPc : public SimFlashFsFlashMemory
{
	std::vector<std::byte> mem;

public:
	// create a new file, if it does not exists.
	// automatically resizes the file to the given size
	SimSTM32InternalFlashPc( const std::string & filename_, std::size_t size );

	// opens a file
	// size will be automatically detected
	SimSTM32InternalFlashPc( const std::string & filename_);

	std::size_t write( std::size_t address, const std::byte *data, std::size_t size ) override;
	std::size_t read( std::size_t address, std::byte *data, std::size_t size ) override;

	void erase( std::size_t address, std::size_t size ) override;

protected:
	void init();

};

} // namespace SimPc
} // namespace SimpleFlashFs

