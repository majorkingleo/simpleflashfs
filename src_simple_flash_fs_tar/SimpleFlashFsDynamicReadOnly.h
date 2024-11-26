/**
 * SimpleFlashFs main implementation class
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */

#pragma once
#include <dynamic/SimpleFlashFsDynamic.h>
#include <SimpleFlashFsConstants.h>

namespace SimpleFlashFs::dynamic {

class SimpleFlashFsReadOnly : public SimpleFlashFs
{
public:
	using base_t = SimpleFlashFs;

public:

	SimpleFlashFsReadOnly( FlashMemoryInterface *mem_interface )
	: SimpleFlashFs( mem_interface )
	{

	}

	virtual void erase_inode_and_unused_pages( base_t::file_handle_t & inode_to_erase, base_t::file_handle_t & next_inode_version ) override {
		// do nothing
	}
};


} // namespace SimpleFlashFs::dynamic
