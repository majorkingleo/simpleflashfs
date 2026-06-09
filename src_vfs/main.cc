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
#include "CommandParser.h"
#include "FilesystemCommands.h"

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

        auto header = ::SimpleFlashFs::dynamic::SimpleFlashFs::create_default_header(DRIVE_A_FM_25_W_256_PAGE_SIZE, DRIVE_A_FM_25_W_256_SIZE / DRIVE_A_FM_25_W_256_PAGE_SIZE );

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
        auto header = ::SimpleFlashFs::dynamic::SimpleFlashFs::create_default_header(DRIVE_B_AT45_DB321E_PAGE_SIZE, DRIVE_B_AT45_DB321E_SIZE / DRIVE_B_AT45_DB321E_PAGE_SIZE );

        if( !::SimpleFlashFs::dynamic::SimpleFlashFs::create(header) ) {
            throw STDERR_EXCEPTION( "cannot create drive b" );
        }
    }
};

class CommandQuit : public SimpleFlashFs::Vfs::Command
{
public:
	SimpleFlashFs::Vfs::CommandResult execute(const std::vector<std::string>& args) override {
		return {true, "OK", "Exiting...\n", true};			
	}

		
	std::string get_description() const override {
		return "Quit the program";
	}	

	std::string get_usage() const override {
		return "quit  - Exit the program";
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

	Arg::StringOption o_cmd("c");
	o_cmd.addName( "cmd" );
	o_cmd.setDescription("execute cmd (splitted by ;)");
	o_cmd.setRequired(false);
	arg.addOptionR( &o_cmd );

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

		vfs->start();

		// Create command parser for filesystem commands
		auto parser = std::make_shared<Vfs::CommandParser>(vfs);

		// Register built-in commands
		parser->register_command("ls", std::make_shared<Vfs::ListCommand>(vfs));
		parser->register_command("dir", std::make_shared<Vfs::ListCommand>(vfs), "");
		parser->register_command("cat", std::make_shared<Vfs::CatCommand>(vfs));
		parser->register_command("cp", std::make_shared<Vfs::CopyCommand>(vfs));
		parser->register_command("mv", std::make_shared<Vfs::MoveCommand>(vfs));
		parser->register_command("rm", std::make_shared<Vfs::RemoveCommand>(vfs));
		parser->register_command("touch", std::make_shared<Vfs::TouchCommand>(vfs));
		parser->register_command("help", std::make_shared<Vfs::HelpCommand>(parser), "?");
        parser->register_command("format", std::make_shared<Vfs::FormatCommand>(vfs));
        parser->register_command("cd", std::make_shared<Vfs::ChangeDirectoryCommand>(vfs));
        parser->register_command("pwd", std::make_shared<Vfs::PrintWorkingDirectoryCommand>(vfs));
		parser->register_command("quit", std::make_shared<CommandQuit>());
		parser->register_command("a:", std::make_shared<Vfs::DOSChangeDriveCommand>(vfs, "a"));
		parser->register_command("b:", std::make_shared<Vfs::DOSChangeDriveCommand>(vfs, "b"));

		bool continue_interactive = true;

		if( o_cmd.isSet() ) {
			std::vector<std::string_view> commands;

			for( const auto & cmd : *o_cmd.getValues() ) {
				auto cmds = Tools::split_string_view( cmd, ";" );
				for( const auto & c : cmds ) {
					commands.push_back(c);
				}
			}

			for( const auto & cmd : commands ) {
				auto result = parser->execute_command_string(std::string(cmd));
				std::cout << result.output;
				if (!result.success) {
					std::cerr << "Error: " << result.message << std::endl;
				}
				if( result.stop_execution ) {
					continue_interactive = false;
					break;
				}
			}
		}


		if( continue_interactive ) {
			std::cout << "SimpleFlashFS VFS Server is running. Enter commands (type 'help' for available commands):" << std::endl;

			// Interactive command loop
			std::string line;
			while( std::getline( std::cin, line ) ) {
				try {
					auto result = parser->execute_command_string(line);
					std::cout << result.output;
					if (!result.success) {
						std::cerr << "Error: " << result.message << std::endl;
					}
					if( result.stop_execution ) {
						break;
					}
				} catch( const std::exception &error ) {
					std::cerr << "Error executing command: " << error.what() << std::endl;
				}		
			}
		}
		
        vfs->stop();

    } catch( const std::exception &error ) {
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}
