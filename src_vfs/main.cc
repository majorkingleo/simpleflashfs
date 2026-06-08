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
#include "FramFsImplDetail.h"
#include "SimpleFlashFsThreadedVfsServer.h"

using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::dynamic;
using namespace SimpleFlashFs::SimPc;

constexpr uint32_t DRIVE_A_FM_25_W_256_SIZE = 0x00007FFF + 1;
constexpr uint32_t DRIVE_A_FM_25_W_256_PAGE_SIZE = 256;

class FramFsDriveA : public FramFsImplDetail
{
public:
    FramFsDriveA( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
    : FramFsImplDetail( mem_interface_, "a" )
    {}

    void create() override {

        auto header = ::SimpleFlashFs::dynamic::SimpleFlashFs::create_default_header(DRIVE_A_FM_25_W_256_SIZE, DRIVE_A_FM_25_W_256_SIZE / DRIVE_A_FM_25_W_256_PAGE_SIZE );

        if( !::SimpleFlashFs::dynamic::SimpleFlashFs::create(header) ) {
            throw STDERR_EXCEPTION( "cannot create drive a" );
        }
    }
};

constexpr uint32_t DRIVE_B_AT45_DB321E_SIZE         = 32'000'000/8;
constexpr uint32_t DRIVE_B_AT45_DB321E_PAGE_SIZE    = 528;

class FramFsDriveB : public FramFsImplDetail
{
public:
    FramFsDriveB( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
    : FramFsImplDetail( mem_interface_, "b" )
    {}

    void create() override {
        auto header = ::SimpleFlashFs::dynamic::SimpleFlashFs::create_default_header(DRIVE_B_AT45_DB321E_SIZE, DRIVE_B_AT45_DB321E_SIZE / DRIVE_B_AT45_DB321E_PAGE_SIZE );

        if( !::SimpleFlashFs::dynamic::SimpleFlashFs::create(header) ) {
            throw STDERR_EXCEPTION( "cannot create drive b" );
        }
    }
};

std::shared_ptr<Vfs::VfsServerInterface>                vfs         = std::make_shared<Vfs::SimpleFlashFsThreadedVfsServer>();
std::shared_ptr<::SimpleFlashFs::FlashMemoryInterface>  mem_drive_a = std::make_shared<SimFlashFsFlashMemory>(".drive_a", DRIVE_A_FM_25_W_256_SIZE);
std::shared_ptr<::SimpleFlashFs::FlashMemoryInterface>  mem_drive_b = std::make_shared<SimFlashFsFlashMemory>(".drive_b", DRIVE_B_AT45_DB321E_SIZE);

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
        
        vfs->register_drive( std::make_shared<FramFsDriveA>(mem_drive_a.get()) );
        vfs->register_drive( std::make_shared<FramFsDriveB>(mem_drive_b.get()) );

    } catch( const std::exception &error ) {
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}
