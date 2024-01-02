/**
 * SimpleFlashFs instance handler
 * @author Copyright (c) 2024 Martin Oberzalek
 */
#pragma once
#ifndef SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICINSTANCEHANDLER_H_
#define SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICINSTANCEHANDLER_H_

#include "SimpleFlashFsDynamic.h"
#include <map>

namespace SimpleFlashFs {

class FlashMemoryInterface;

namespace dynamic {

class InstanceHandler
{
	std::map<std::string,std::shared_ptr<SimpleFlashFs>> instances;
public:

	void register_instance(const std::string & name, std::shared_ptr<SimpleFlashFs> & instance );
	void deregister_instance(const std::string & name );

	std::shared_ptr<SimpleFlashFs> get( const std::string & name ) {
		return instances[name];
	}

	static InstanceHandler & instance();
};

} // namespace dynamic
} // namespace SimpleFlashFs

#endif /* SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICINSTANCEHANDLER_H_ */
