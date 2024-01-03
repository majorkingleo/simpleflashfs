/**
 * SimpleFlashFs instance handler
 * @author Copyright (c) 2024 Martin Oberzalek
 */

#include "SimpleFlashFsDynamicInstanceHandler.h"

namespace SimpleFlashFs {
namespace dynamic {

InstanceHandler & InstanceHandler::instance()
{
	static InstanceHandler inst;
	return inst;
}

void InstanceHandler::register_instance(const std::string & name, std::shared_ptr<SimpleFlashFs> & instance )
{
	instances[name] = instance;
}

void InstanceHandler::deregister_instance(const std::string & name )
{
	if( auto it = instances.find( name ); it != instances.end() ) {
		instances.erase(it);
	}
}

} // namespace dynamic
} // namespace SimpleFlashFs

