/**
 * SimpleFlashFs instance handler
 * @author Copyright (c) 2024 Martin Oberzalek
 */

#include "SimpleFlashFsDynamicInstanceHandler.h"
#include <CpputilsDebug.h>
#include <format.h>

using namespace Tools;

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

std::shared_ptr<SimpleFlashFs> InstanceHandler::get( const std::string & name ) {

	/*
	for( auto & pair : instances ) {
		CPPDEBUG( format( "instance: '%s'", pair.first ) );
	}*/

	auto ret = instances[name];

	if( !ret ) {
		CPPDEBUG( format( "%p no instance with name: '%s'", this, name ) );
	}

	return ret;
}

void InstanceHandler::deregister_instance(const std::string & name )
{
	//CPPDEBUG( format( "deregistering '%s'", name ));
	if( auto it = instances.find( name ); it != instances.end() ) {
		instances.erase(it);
	}
}

} // namespace dynamic
} // namespace SimpleFlashFs

