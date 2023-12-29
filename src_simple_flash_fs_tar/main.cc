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
#include <ColBuilder.h>
#include <dynamic/SimpleFlashFsDynamic.h>
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>
#include <SimpleFlashFsConstants.h>
#include <string_utils.h>
#include <filesystem>

using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::dynamic;
using namespace SimpleFlashFs::SimPc;

namespace std {
	std::ostream & operator<<( std::ostream & out, SimpleFlashFs::dynamic::Header::ENDIANESS en )
	{
		switch( en )
		{
		case Header::ENDIANESS::BE: return out << ENDIANESS_BE;
		case Header::ENDIANESS::LE: return out << ENDIANESS_LE;
		}

		return out;
	}
}

namespace std {
	std::ostream & operator<<( std::ostream & out, SimpleFlashFs::dynamic::Header::CRC_CHECKSUM crc )
	{
		switch( crc )
		{
		case Header::CRC_CHECKSUM::CRC32: return out << "CRC32";
		}

		return out;
	}
}


static void info_fs( const SimpleFlashFs::dynamic::SimpleFlashFs & fs )
{
	{
		ColBuilder co;
		std::cout << "Header:\n";

		const int BYTES = co.addCol("Bytes");
		const int LEN = co.addCol("Len");
		const int TITLE = co.addCol("Title");
		const int DATA = co.addCol("Data");

		const SimpleFlashFs::dynamic::Header & header = fs.get_header();

		co.addColData(BYTES, "00 - 12" );
		co.addColData(LEN,   x2s(MAGICK_STRING_LEN) );
		co.addColData(TITLE, "Magic");
		co.addColData(DATA,  header.magic_string );

		co.addColData(BYTES, "13 - 14" );
		co.addColData(TITLE, "Endianess");
		co.addColData(LEN,   x2s(ENDIANESS_LEN) );
		co.addColData(DATA,  x2s(header.endianess) );

		co.addColData(BYTES, "15 - 16" );
		co.addColData(TITLE, "Version");
		co.addColData(LEN,   x2s(2) );
		co.addColData(DATA,  x2s(header.version) );

		co.addColData(BYTES, "17 - 20" );
		co.addColData(TITLE, "page size");
		co.addColData(LEN,   x2s(4) );
		co.addColData(DATA,  x2s(header.page_size) );

		co.addColData(BYTES, "21 - 28" );
		co.addColData(TITLE, "filesystem size");
		co.addColData(LEN,   x2s(8) );
		co.addColData(DATA,  x2s(header.filesystem_size) );

		co.addColData(BYTES, "29 - 32" );
		co.addColData(TITLE, "max inodes");
		co.addColData(LEN,   x2s(4) );
		co.addColData(DATA,  x2s(header.max_inodes) );

		co.addColData(BYTES, "33 - 34" );
		co.addColData(TITLE, "max path len");
		co.addColData(LEN,   x2s(2) );
		co.addColData(DATA,  x2s(header.max_path_len) );

		co.addColData(BYTES, "35 - 36" );
		co.addColData(TITLE, "crc checksum type");
		co.addColData(LEN,   x2s(2) );
		co.addColData(DATA,  x2s(header.crc_checksum_type) );


		const unsigned width = co.get_width();
		for( unsigned i = 0; i < width; i++ ) {
			std::cout << '=';
		}
		std::cout << '\n';

		std::cout << co.toString() << std::endl;
	}

	{
		ColBuilder co;
		const int INFO = co.addCol("Info");
		const int DATA = co.addCol("Data");

		co.addColData(INFO, "free data pages" );
		co.addColData(DATA, x2s(fs.get_number_of_free_data_pages()));

		co.addColData(INFO, "max inode number" );
		co.addColData(DATA, x2s(fs.get_max_inode_number()));

		std::cout << co.toString() << std::endl;
	}
}

static std::vector<std::byte> read_file( const std::string & file )
{
	std::ifstream in( file, std::ios_base::in | std::ios_base::binary );
	if( !in ) {
		throw STDERR_EXCEPTION( format( "cannot open file '%s'", file ) );
	}

	std::vector<std::byte> data( std::filesystem::file_size(file) );

	in.read(reinterpret_cast<char*>(&data[0]),data.size());

	return data;
}

static void add_file( SimpleFlashFs::dynamic::SimpleFlashFs & fs, const std::string & file )
{
	auto data = read_file( file );
	auto handle = fs.open( file, std::ios_base::in | std::ios_base::out );
	handle->write( data.data(), data.size() );
}

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

	Arg::StringOption o_fs_info("i");
	o_fs_info.addName( "info" );
	o_fs_info.setDescription("inspect a filesystem");
	o_fs_info.setRequired(false);
	arg.addOptionR( &o_fs_info );

	Arg::StringOption o_fs_add("a");
	o_fs_add.addName( "add" );
	o_fs_add.setDescription("add file");
	o_fs_add.setRequired(false);
	arg.addOptionR( &o_fs_add );

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

		if( o_fs_info.isSet() ) {
			std::string file = o_fs_info.getValues()->at(0);

			SimFlashFsFlashMemoryInterface mem(file);
			SimpleFlashFs::dynamic::SimpleFlashFs fs(&mem);

			if( !fs.init() ) {
				throw STDERR_EXCEPTION( "init failed" );
			}

			info_fs( fs );
		}


		if( o_fs_add.isSet() ) {
			auto values = o_fs_add.getValues();

			std::string file = values->at(0);

			SimFlashFsFlashMemoryInterface mem(file);
			SimpleFlashFs::dynamic::SimpleFlashFs fs(&mem);

			if( !fs.init() ) {
				throw STDERR_EXCEPTION( "init failed" );
			}

			for( unsigned i = 1; i < values->size(); i++ ) {
				add_file( fs, values->at(i) );
			}
		}

	} catch( const std::exception &error ) {
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}
