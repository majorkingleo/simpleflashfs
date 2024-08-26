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
#include <optional>

using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::dynamic;
using namespace SimpleFlashFs::SimPc;

namespace std {
	std::ostream & operator<<( std::ostream & out, SimpleFlashFs::dynamic::SimpleFlashFs::Header::ENDIANESS en )
	{
		using Header = SimpleFlashFs::dynamic::SimpleFlashFs::Header;

		switch( en )
		{
		case Header::ENDIANESS::BE: return out << ENDIANESS_BE;
		case Header::ENDIANESS::LE: return out << ENDIANESS_LE;
		}

		return out;
	}
}

namespace std {
	std::ostream & operator<<( std::ostream & out, SimpleFlashFs::dynamic::SimpleFlashFs::Header::CRC_CHECKSUM crc )
	{
		using Header = SimpleFlashFs::dynamic::SimpleFlashFs::Header;

		switch( crc )
		{
		case Header::CRC_CHECKSUM::CRC32: return out << "CRC32";
		}

		return out;
	}
}


static void info_fs( SimpleFlashFs::dynamic::SimpleFlashFs & fs )
{
	using Header = SimpleFlashFs::dynamic::SimpleFlashFs::Header;

	{
		ColBuilder co;
		std::cout << "Header:\n";

		const int BYTES = co.addCol("Bytes");
		const int LEN = co.addCol("Len");
		const int TITLE = co.addCol("Title");
		const int DATA = co.addCol("Data");
		const int UNIT = co.addCol("Unit");

		const Header & header = fs.get_header();

		co.addColData(BYTES, "00 - 12" );
		co.addColData(LEN,   x2s(MAGICK_STRING_LEN) );
		co.addColData(TITLE, "Magic");
		co.addColData(DATA,  header.magic_string );
		co.addColData(UNIT,  "" );

		co.addColData(BYTES, "13 - 14" );
		co.addColData(TITLE, "Endianess");
		co.addColData(LEN,   x2s(ENDIANESS_LEN) );
		co.addColData(DATA,  x2s(header.endianess) );
		co.addColData(UNIT,  "" );

		co.addColData(BYTES, "15 - 16" );
		co.addColData(TITLE, "Version");
		co.addColData(LEN,   x2s(2) );
		co.addColData(DATA,  x2s(header.version) );
		co.addColData(UNIT,  "" );

		co.addColData(BYTES, "17 - 20" );
		co.addColData(TITLE, "page size");
		co.addColData(LEN,   x2s(4) );
		co.addColData(DATA,  x2s(header.page_size) );
		co.addColData(UNIT,  "bytes" );

		co.addColData(BYTES, "21 - 28" );
		co.addColData(TITLE, "filesystem size");
		co.addColData(LEN,   x2s(8) );
		co.addColData(DATA,  x2s(header.filesystem_size));
		co.addColData(UNIT,  "pages" );

		co.addColData(BYTES, "29 - 32" );
		co.addColData(TITLE, "max inodes");
		co.addColData(LEN,   x2s(4) );
		co.addColData(DATA,  x2s(header.max_inodes) );
		co.addColData(UNIT,  "" );

		co.addColData(BYTES, "33 - 34" );
		co.addColData(TITLE, "max path len" );
		co.addColData(LEN,   x2s(2) );
		co.addColData(DATA,  x2s(header.max_path_len) );
		co.addColData(UNIT,  "bytes" );

		co.addColData(BYTES, "35 - 36" );
		co.addColData(TITLE, "crc checksum type");
		co.addColData(LEN,   x2s(2) );
		co.addColData(DATA,  x2s(header.crc_checksum_type) );
		co.addColData(UNIT,  "" );


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

	{
		ColBuilder co;
		const int INODE      = co.addCol("Inode");
		const int PAGE       = co.addCol("Page");
		const int FILENAME   = co.addCol("Filename");
		const int SIZE       = co.addCol("Size");
		const int DATA_PAGES = co.addCol("Data pages");

		for( auto inode : fs.get_all_inodes() ) {
			co.addColData(INODE,      format( "%d,%d", inode->inode.inode_number, inode->inode.inode_version_number ));
			co.addColData(PAGE,       x2s(inode->page));
			co.addColData(FILENAME,   inode->inode.file_name);
			co.addColData(SIZE,       x2s(inode->inode.file_len));
			co.addColData(DATA_PAGES, IterableToCommaSeparatedString(inode->inode.data_pages));
		}

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
	auto handle = fs.open( file, std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
	handle.write( data.data(), data.size() );
}

static bool extract_file( SimpleFlashFs::dynamic::SimpleFlashFs & fs, const std::string & file )
{
	auto handle = fs.open( file, std::ios_base::in );

	if( !handle ) {
		CPPDEBUG( "file not found" );
		return false;
	}

	std::vector<std::byte> buffer(handle.inode.file_len);

	if( handle.read(buffer.data(),buffer.size()) != buffer.size() ) {
		CPPDEBUG( "reading all data failed" );
		return false;
	}

	std::ofstream out( file, std::ios_base::trunc | std::ios_base::binary );

	if( !out ) {
		CPPDEBUG( "cannot open file for writing" );
		return false;
	}

	out.write(reinterpret_cast<const char*>(buffer.data()),buffer.size());
	if( !out ) {
		return false;
	}

	return true;
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

	Arg::StringOption o_tar_archive("f");
	o_tar_archive.addName( "file" );
	o_tar_archive.setDescription("archive file");
	o_tar_archive.setRequired(false);
	o_tar_archive.setMinValues(1);
	o_tar_archive.setMaxValues(1);
	arg.addOptionR( &o_tar_archive );

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

	Arg::FlagOption o_tar_list("t");
	o_tar_list.addName( "list" );
	o_tar_list.setDescription("list files in archive");
	o_tar_list.setRequired(false);
	arg.addOptionR( &o_tar_list );

	Arg::StringOption o_tar_extract("x");
	o_tar_extract.addName( "extract" );
	o_tar_extract.setDescription("extract files from archive");
	o_tar_extract.setRequired(false);
	o_tar_extract.setMinValues(0);
	arg.addOptionR( &o_tar_extract );

	try {

		std::optional<std::string> archive_file_name;

		if( !arg.parse() )
		{
			std::cout << arg.getHelp(5,20,30, 80 ) << std::endl;
			return 1;
		}

		if( o_tar_archive.isSet() ) {
			archive_file_name = *o_tar_archive.getValues()->begin();
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

		if( o_tar_list.isSet() ) {

			if( !archive_file_name ) {
				throw STDERR_EXCEPTION( "missing archive file name");
			}

			SimFlashFsFlashMemoryInterface mem(*archive_file_name);
			SimpleFlashFs::dynamic::SimpleFlashFs fs(&mem);

			if( !fs.init() ) {
				throw STDERR_EXCEPTION( "init failed" );
			}

			for( auto inode : fs.get_all_inodes() ) {
				std::cout << inode->inode.file_name << std::endl;
			}
		}

		if( o_tar_extract.isSet() ) {

			if( !archive_file_name ) {
				throw STDERR_EXCEPTION( "missing archive file name");
			}

			SimFlashFsFlashMemoryInterface mem(*archive_file_name);
			SimpleFlashFs::dynamic::SimpleFlashFs fs(&mem);

			if( !fs.init() ) {
				throw STDERR_EXCEPTION( "init failed" );
			}

			auto values = o_tar_extract.getValues();
			std::set<std::string> v(values->begin(), values->end());
			std::size_t count = 0;

			for( auto inode : fs.get_all_inodes() ) {

				const std::string & file_name = inode->inode.file_name;

				if( !v.empty() ) {
					if( v.find(file_name) == v.end() ) {
						continue;
					}
				}

				if( !extract_file(fs,file_name) ) {
					throw STDERR_EXCEPTION( format( "Cannot extract file: %s", file_name ) );
				}

				count++;
			} // for

			if( !v.empty() && count != v.size() ) {
				std::cerr << "warning: no all files found in archive\n";
			}
		}

	} catch( const std::exception &error ) {
		std::cerr << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}
