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

} // namespace dynamic
} // namespace SimpleFlashFs

