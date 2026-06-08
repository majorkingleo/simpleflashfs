#include <arg.h>
#include <iostream>
#include <OutDebug.h>
#include <memory>
#include <format.h>
#include <ColBuilder.h>
#include <dynamic/SimpleFlashFsDynamic.h>
#include <stderr_exception.h>
#include <SimpleFlashFsConstants.h>
#include <string_utils.h>
#include <filesystem>
#include <optional>
#include <set>
#include "../src/sim_pc/SimFlashMemoryPc.h"


using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::dynamic;
using namespace SimpleFlashFs::SimPc;

int main( int argc, char **argv )
{
	ColoredOutput co;

	Arg::Arg arg( argc, argv );
	arg.addPrefix( "-" );
	arg.addPrefix( "--" );

	Arg::OptionChain oc_info;
	arg.addChainR(&oc_info);
	oc_info.setMinMatch(1);
	oc_info.setContinueOnMatch( false );
	oc_info.setContinueOnFail( true );

	Arg::FlagOption o_help( "help" );
	o_help.setDescription( "Show this page" );
	oc_info.addOptionR( &o_help );

	Arg::FlagOption o_debug("d");
	o_debug.addName( "debug" );
	o_debug.setDescription("print debugging messages");
	o_debug.setRequired(false);
	arg.addOptionR( &o_debug );

	return 0;
}
