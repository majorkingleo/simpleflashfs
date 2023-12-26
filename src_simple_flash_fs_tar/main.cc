/**
 * tar like command line tool
 * for creating, extracting an modifyinf simpleflashfs
 * @author Copyright (c) 2023 - 2024 Martin Oberzalek
 */

#include <arg.h>
#include <iostream>
#include <OutDebug.h>
#include <memory>
#include <format.h>
#include <dynamic/SimpleFlashFsDynamic.h>
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>


using namespace Tools;
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

	Arg::StringOption o_create("c");
	o_create.addName( "create" );
	o_create.setDescription("create new filesystem");
	o_create.setRequired(false);
	arg.addOptionR( &o_create );

	try {

		if( !arg.parse() )
		{
			std::cout << arg.getHelp(5,20,30, 80 ) << std::endl;
			return 1;
		}

		if( o_debug.getState() )
		{
			Tools::x_debug = new OutDebug();
		}

		if( o_help.getState() ) {
			std::cout << arg.getHelp(5,20,30, 80 ) << std::endl;
			return 1;
		}

		if( o_create.isSet() ) {
			const std::size_t size = 100*1024;

			std::string file = o_create.getValues()->at(0);
			SimFlashFsFlashMemoryInterface mem(file,size);
			SimpleFlashFs::dynamic::SimpleFlashFs fs(&mem);

			const std::size_t page_size = 528;

			if( !fs.create(fs.create_default_header(page_size, size/page_size)) ) {
				throw STDERR_EXCEPTION( format( "cannot create %s", file ) );
			}
		}

	} catch( const std::exception &error ) {
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}
