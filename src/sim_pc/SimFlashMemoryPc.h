/*
 * SimFlashMemoryInterfacePc.h
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */

#ifndef SRC_SIM_PC_SIMFLASHMEMORYPC_H_
#define SRC_SIM_PC_SIMFLASHMEMORYPC_H_

#include "../SimpleFlashFsFlashMemoryInterface.h"

#include <string>
#include <fstream>

namespace SimpleFlashFs {
namespace SimPc {

class SimFlashFsFlashMemory : public FlashMemoryInterface
{
protected:
	std::string filename;
	std::fstream file;
	std::size_t file_size;

public:
	// create a new file, if it does not exists.
	// automatically resizes the file to the given size
	SimFlashFsFlashMemory( const std::string & filename_, std::size_t size );

	// opens a file
	// size will be automatically detected
	SimFlashFsFlashMemory( const std::string & filename_);

	std::size_t size() const override {
		return file_size;
	}

	std::size_t write( std::size_t address, const std::byte *data, std::size_t size ) override;
	std::size_t read( std::size_t address, std::byte *data, std::size_t size ) override;

	void erase( std::size_t address, std::size_t size ) override;

};

} // namespace SimPc
} // namespace SimpleFlashFs


#endif /* SRC_SIM_PC_SIMFLASHMEMORYPC_H_ */
