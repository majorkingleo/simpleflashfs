/*
 * SimpleFlashFsFlashMemoryInterface.h
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */

#ifndef SRC_SIMPLEFLASHFSFLASHMEMORYINTERFACE_H_
#define SRC_SIMPLEFLASHFSFLASHMEMORYINTERFACE_H_

#include <cstddef>

namespace SimpleFlashFs {

class FlashMemoryInterface
{
public:
	virtual ~FlashMemoryInterface() {}

	virtual std::size_t size() const = 0;

	virtual std::size_t write( std::size_t address, const std::byte *data, std::size_t size ) = 0;
	virtual std::size_t read( std::size_t address, std::byte *data, std::size_t size ) = 0;

	virtual void erase( std::size_t address, std::size_t size ) = 0;
};


} // namespace SimpleFlashFs


#endif /* SRC_SIMPLEFLASHFSFLASHMEMORYINTERFACE_H_ */
