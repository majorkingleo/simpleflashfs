/*
 * test_H7TwoFace.cc
 *
 *  Created on: 29.08.2024
 *      Author: martin.oberzalek
 */
#include "test_H7TwoFace.h"
#include "../src_2face/H7TwoFace.h"
#include "../src_2face/H7TwoFaceConfig.h"
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>
#include <format.h>

using namespace Tools;

namespace {

class TestCaseH7Base : public TestCaseBase<bool>
{
protected:
	std::optional<::SimpleFlashFs::SimPc::SimFlashFsFlashMemoryInterface> mem1;
	std::optional<::SimpleFlashFs::SimPc::SimFlashFsFlashMemoryInterface> mem2;

public:
	TestCaseH7Base( const std::string & name_,
			bool expected_result_ = true,
			bool exception_ = false )
	: TestCaseBase<bool>( name_, expected_result_, exception_ )
	{

	}


	void init()
	{
		const std::string file1 = "test_h7_page1.bin";
		const std::string file2 = "test_h7_page2.bin";
		mem1.emplace(file1,SFF_MAX_SIZE);
		mem2.emplace(file2,SFF_MAX_SIZE);
		H7TwoFace::set_memory_interface(&mem1.value(),&mem2.value());
	}

	void deinit() {
		H7TwoFace::set_memory_interface(nullptr,nullptr);
		mem1.reset();
		mem2.reset();
	}
};


class TestCaseH7FuncNoInp : public TestCaseH7Base
{
	typedef std::function<bool()> Func;
	Func func;

public:
	TestCaseH7FuncNoInp( const std::string & name,
			bool expected_result_,
			Func func_ )
	: TestCaseH7Base( name, expected_result_ ),
	  func( func_ )
	  {}

	bool run() override {
		try {
			init();
			bool ret = func();
			deinit();
			return ret;

		} catch( const std::exception & error ) {
			deinit();
			throw error;

		} catch( ... ) {
			deinit();
			throw;
		}
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_write1()
{
	return std::make_shared<TestCaseH7FuncNoInp>(__FUNCTION__, true, []() {

		auto f = H7TwoFace::open( "2Face.write1", std::ios_base::in | std::ios_base::out );

		if( !f ) {
			CPPDEBUG( "cannot open file" );
			return false;
		}

		char buffer[1000] = { "Hello" };

		std::size_t bytes_written = f->write( reinterpret_cast<std::byte*>(buffer), sizeof(buffer) );
		if( bytes_written != sizeof(buffer) ) {
			CPPDEBUG( format( "%d bytes written", bytes_written ) );
			return false;
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_write2()
{
	return std::make_shared<TestCaseH7FuncNoInp>(__FUNCTION__, true, []() {

		// test opening another handle later
		{
			auto f = H7TwoFace::open( "2Face.write1", std::ios_base::in | std::ios_base::out );

			if( !f ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}
		}

		{
			auto f = H7TwoFace::open( "2Face.write2", std::ios_base::in | std::ios_base::out );

			if( !f ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_write3()
{
	return std::make_shared<TestCaseH7FuncNoInp>(__FUNCTION__, true, []() {

		// test opening another handle later
		auto f1 = H7TwoFace::open( "2Face.write1", std::ios_base::in );

		if( !f1 ) {
			CPPDEBUG( "cannot open file" );
			return false;
		}

		auto f2 = H7TwoFace::open( "2Face.write2", std::ios_base::in );

		// should not work, because instance f1 is still there
		if( !f2 ) {
			return true;
		}

		return true;
	});
}


std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_delete1()
{
	return std::make_shared<TestCaseH7FuncNoInp>(__FUNCTION__, true, []() {

		// test deleting a file
		{
			auto f1 = H7TwoFace::open( "2Face.delete1", std::ios_base::out );

			if( !f1 ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}
		}

		{
			auto f2 = H7TwoFace::open( "2Face.delete1", std::ios_base::in );

			// s	hould not work, because instance f1 is still there
			if( !f2 ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}

			if( !f2->delete_file() ) {
				return false;
			}

		}

		{
			auto f2 = H7TwoFace::open( "2Face.delete1", std::ios_base::in );

			if( !f2 ) {
				return true;
			}
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_rename1()
{
	return std::make_shared<TestCaseH7FuncNoInp>(__FUNCTION__, true, []() {

		// test renaming a file
		{
			auto f1 = H7TwoFace::open( "2Face.rename1", std::ios_base::out );

			if( !f1 ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}
		}

		{
			auto f2 = H7TwoFace::open( "2Face.rename1", std::ios_base::in );

			// should work, because instance "2Face.rename1" is still there
			if( !f2 ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}

			if( !f2->rename_file("2Face.rename1.hias") ) {
				CPPDEBUG( "rename failed" );
				return false;
			}
		}

		{
			auto f3 = H7TwoFace::open( "2Face.rename1", std::ios_base::in );

			if( !f3 ) {
				CPPDEBUG( "ok" );
			} else {
				CPPDEBUG( "file not renamed" );
				return false;
			}
		}

		{
			auto f2 = H7TwoFace::open( "2Face.rename1.hias", std::ios_base::in );

			if( !f2 ) {
				CPPDEBUG( "file not renamed" );
				return false;
			}
		}

		return true;
	});
}

static void clear_fs()
{
	auto file_names = H7TwoFace::list_files();

	for( auto & file_name : file_names ) {
		 auto file = H7TwoFace::open( file_name, std::ios_base::in );
		 file->delete_file();
	}
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_max_files1()
{
	return std::make_shared<TestCaseH7FuncNoInp>(__FUNCTION__, true, []() {


		return true;
	});
}


std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_list_files1()
{
	return std::make_shared<TestCaseH7FuncNoInp>(__FUNCTION__, true, []() {
		clear_fs();

		std::set<std::string> file_names;

		for( unsigned i = 0; i < 10; i++ ) {
			file_names.insert( Tools::format( "file#%02d.txt", i ) );
		}

		for( auto & file_name : file_names ) {
			auto file = H7TwoFace::open( file_name, std::ios_base::out );
		}

		auto file_names_in_fs = H7TwoFace::list_files();

		for( auto & file_name_in_fs : file_names_in_fs ) {
			if( !file_names.count( std::string(file_name_in_fs) ) ) {
				CPPDEBUG( Tools::format( "file '%s' not found") );
				return false;
			}
		}

		return true;
	});
}
