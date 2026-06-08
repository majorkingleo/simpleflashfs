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


class FramFsDriveA : public FramFsImplDetail
{
public:
    FramFsDriveA( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
    : FramFsImplDetail( mem_interface_, "a" )
    {}

    void create() override {

        static constexpr const std::size_t FRAMFS_FILE_NAME_MAX = 30;
        static constexpr const std::size_t FRAMFS_PAGE_SIZE = 256;
        static constexpr const std::size_t FRAMFS_MAX_SIZE = 30*1024;
        static constexpr const std::size_t FRAMFS_MAX_NUMBER_OF_FILES = FRAMFS_MAX_SIZE / FRAMFS_PAGE_SIZE / 10 * 2;

        // SFF_MAX_SIZE / SFF_PAGE_SIZE is the maximum file size that fits on a H7internal flash page
        // so 128*1024/512 = 256 is a good value.
        // sizeof(FileHandle) ~ SFF_MAX_PAGES * uint32_t(4) + SFF_FILE_NAME_MAX + SFF_PAGE_SIZE
        static constexpr const std::size_t FRAMFS_MAX_PAGES = 256;

        auto header = ::SimpleFlashFs::dynamic::SimpleFlashFs::create_default_header(FRAMFS_PAGE_SIZE, FRAMFS_MAX_SIZE / FRAMFS_PAGE_SIZE );

        if( !::SimpleFlashFs::dynamic::SimpleFlashFs::create(header) ) {
            throw STDERR_EXCEPTION( "cannot create drive a" );
        }
    }
};

class FramFsDriveB : public FramFsImplDetail
{
public:
    FramFsDriveB( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
    : FramFsImplDetail( mem_interface_, "b" )
    {}

    void create() override {
        const std::size_t size = 100*1024;
        const std::size_t page_size = 528;

        if( !::SimpleFlashFs::dynamic::SimpleFlashFs::create(::SimpleFlashFs::dynamic::SimpleFlashFs::create_default_header(page_size, size/page_size)) ) {
            throw STDERR_EXCEPTION( "cannot create drive a" );
        }
    }
};

std::shared_ptr<Vfs::VfsServerInterface>    vfs         = std::make_shared<Vfs::SimpleFlashFsThreadedVfsServer>();
std::shared_ptr<::SimpleFlashFs::FlashMemoryInterface>      mem_drive_a = std::make_shared<SimFlashFsFlashMemory>(".drive_a", 1024*1024);
std::shared_ptr<::SimpleFlashFs::FlashMemoryInterface>      mem_drive_b = std::make_shared<SimFlashFsFlashMemory>(".drive_b", 1024*1024);

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
