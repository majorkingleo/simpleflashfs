bin_PROGRAMS=\
	simple_flash_fs_tar \
	test_dynamic
		
simple_flash_fs_tar_SOURCES=\
		src_simple_flash_fs_tar/main.cc \
		tools_config.h
		
test_dynamic_SOURCES=\
		src_test_dynamic/test_dynamic.cc \
		src_test_dynamic/test_dynamic_init.h \
		src_test_dynamic/test_dynamic_init.cc \
		src_test_dynamic/test_dynamic_wrapper.h \
		src_test_dynamic/test_dynamic_wrapper.cc \
		tools_config.h		
		

AM_CPPFLAGS = -I$(top_srcdir)/tools \
	-I$(top_srcdir)/cpputils/cpputilsshared  \
	-I$(top_srcdir)/cpputils/cpputilsshared/cpputilsformat \
	-I$(top_srcdir)/cpputils/io \
	-I$(top_srcdir)/common \
	-I$(top_srcdir)/src \
	-std=gnu++20 \
	-D_GLIBCXX_USE_CXX11_ABI=1 \
	-D_XOPEN_SOURCE=700 # for cyggwin fileno()

noinst_LIBRARIES=cpputils/io/libcpputilsio.a \
	cpputils/cpputilsshared/libcpputilsshared.a \
	cpputils/cpputilsshared/cpputilsformat/libcpputilsformat.a \
	common/libcommon.a \
	src/libSimpleFlashFs.a
	
src_libSimpleFlashFs_a_SOURCES=\
		src/crc/crc.c \
		src/crc/crc.h \
		src/dynamic/SimpleFlashFsDynamicHeader.h \
		src/dynamic/SimpleFlashFsDynamic.h \
		src/dynamic/SimpleFlashFsDynamic.cc \
		src/dynamic/SimpleFlashFsDynamicInstanceHandler.h \
		src/dynamic/SimpleFlashFsDynamicInstanceHandler.cc \
		src/dynamic/SimpleFlashFsDynamicWrapper.h \
		src/dynamic/SimpleFlashFsDynamicWrapper.cc \
		src/SimpleFlashFsFlashMemoryInterface.h \
		src/SimpleFlashFsConstants.h \
		src/SimpleFlashFsConstants.cc \
		src/sim_pc/SimFlashMemoryInterfacePc.h \
		src/sim_pc/SimFlashMemoryInterfacePc.cc

cpputils_io_libcpputilsio_a_SOURCES= \
	cpputils/io/ColoredOutput.h \
	cpputils/io/ColoredOutput.cc \
	cpputils/io/CpputilsDebug.cc \
	cpputils/io/CpputilsDebug.h \
	cpputils/io/DetectLocale.h \
	cpputils/io/DetectLocale.cc \
	cpputils/io/OutDebug.h \
	cpputils/io/OutDebug.cc \
	cpputils/io/read_file.h \
	cpputils/io/read_file.cc


cpputils_cpputilsshared_libcpputilsshared_a_SOURCES= \
		cpputils/cpputilsshared/arg.h \
		cpputils/cpputilsshared/arg.cc \
		cpputils/cpputilsshared/backtrace.h \
		cpputils/cpputilsshared/backtrace.cc \
		cpputils/cpputilsshared/cppdir.h \
		cpputils/cpputilsshared/cppdir.cc \
		cpputils/cpputilsshared/file_option.h \
		cpputils/cpputilsshared/file_option.cc \
		cpputils/cpputilsshared/iterator.h \
		cpputils/cpputilsshared/leoini.h \
		cpputils/cpputilsshared/leoini.cc \
		cpputils/cpputilsshared/map_utils.h \
		cpputils/cpputilsshared/pairs.h \
		cpputils/cpputilsshared/pairs.cc \
		cpputils/cpputilsshared/range.h \
		cpputils/cpputilsshared/ref.h \
		cpputils/cpputilsshared/stderr_exception.h \
		cpputils/cpputilsshared/stderr_exception.cc \
		cpputils/cpputilsshared/string_utils.h \
		cpputils/cpputilsshared/string_utils.cc \
		cpputils/cpputilsshared/utf8.h \
		cpputils/cpputilsshared/utf8/core.h \
		cpputils/cpputilsshared/utf8/checked.h \
		cpputils/cpputilsshared/utf8/unchecked.h \
		cpputils/cpputilsshared/utf8/cpp11.h \
		cpputils/cpputilsshared/utf8/cpp17.h \
		cpputils/cpputilsshared/utf8_util.h \
		cpputils/cpputilsshared/utf8_util.cc \
		cpputils/cpputilsshared/xml.h \
		cpputils/cpputilsshared/xml.cc \
		cpputils/cpputilsshared/CyclicArray.h \
		cpputils/cpputilsshared/static_vector.h \
		cpputils/cpputilsshared/static_list.h \
		cpputils/cpputilsshared/static_string.h


cpputils_cpputilsshared_cpputilsformat_libcpputilsformat_a_SOURCES= \
	cpputils/cpputilsshared/cpputilsformat/format.h \
	cpputils/cpputilsshared/cpputilsformat/format1.cc \
	cpputils/cpputilsshared/cpputilsformat/format2.cc \
	cpputils/cpputilsshared/cpputilsformat/format2.h \
	cpputils/cpputilsshared/cpputilsformat/format1.h \
	cpputils/cpputilsshared/cpputilsformat/cformat.h \
	cpputils/cpputilsshared/cpputilsformat/wformat2.cc \
	cpputils/cpputilsshared/cpputilsformat/wformat2.h \
	cpputils/cpputilsshared/cpputilsformat/cwformat.h \
	cpputils/cpputilsshared/cpputilsformat/cwformat.cc
	
	
	
common_libcommon_a_SOURCES=\
	common/TestUtils.h \
	common/ColBuilder.h \
	common/ColBuilder.cc

simple_flash_fs_tar_LDADD = \
	src/libSimpleFlashFs.a \
	cpputils/io/libcpputilsio.a \
	cpputils/cpputilsshared/libcpputilsshared.a \
	common/libcommon.a \
	cpputils/cpputilsshared/cpputilsformat/libcpputilsformat.a
	
test_dynamic_LDADD = \
	src/libSimpleFlashFs.a \
	cpputils/io/libcpputilsio.a \
	cpputils/cpputilsshared/libcpputilsshared.a \
	common/libcommon.a \
	cpputils/cpputilsshared/cpputilsformat/libcpputilsformat.a
		 
LIBS=
    
AM_LDFLAGS=
    
if MINGW
#AM_LDFLAGS += -mwindows
LIBS += -liconv
# with -std=c++11 WIN32 is disabled (_WIN32) would be the valid one, so we define it in mingw.
AM_CPPFLAGS += -DWIN32
else
#AM_LDFLAGS += -lX11
endif

if CYGWIN
LIBS += -liconv
endif

    

