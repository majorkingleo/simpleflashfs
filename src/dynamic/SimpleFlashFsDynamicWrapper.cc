#include "SimpleFlashFsDynamicWrapper.h"
#include "SimpleFlashFsDynamicInstanceHandler.h"
#include <cstring>

using namespace SimpleFlashFs;
using namespace SimpleFlashFs::dynamic;

struct SIMPLE_FLASH_FS_DYNAMIC_FILE
{
	std::shared_ptr<FileHandle> handle;

	SIMPLE_FLASH_FS_DYNAMIC_FILE( std::shared_ptr<FileHandle> h )
	: handle( h )
	{}
};

static std::string default_instance_name;
static std::map<SIMPLE_FLASH_FS_DYNAMIC_FILE*,std::shared_ptr<SIMPLE_FLASH_FS_DYNAMIC_FILE>> file_handles;

int SIMPLE_FLASH_FS_DYNAMIC_EOF = -1;

void SimpleFlashFs_dynamic_instance_name( const char *name )
{
	default_instance_name = name;
}

SIMPLE_FLASH_FS_DYNAMIC_FILE* SimpleFlashFs_dynamic_fopen( const char *path, const char *cmode)
{
	std::shared_ptr<SimpleFlashFs::dynamic::SimpleFlashFs> fs = InstanceHandler::instance().get(default_instance_name);

	if( !fs ) {
		return nullptr;
	}

	std::ios_base::openmode mode = {};

	for( const char* m = cmode; m != 0; m++ ) {
		switch( *m )
		{
		case 'r': mode = std::ios_base::in; break;
		case 'w': mode = std::ios_base::out; break;
		case '+':
			if( mode & std::ios_base::out | std::ios_base::app ) {
				mode = std::ios_base::trunc;
			}
			break;
		case 'a': mode = std::ios_base::app; break;
		}
	}

	auto handle = fs->open( path, mode );
	if( !handle ) {
		return nullptr;
	}

	auto file = std::make_shared<SIMPLE_FLASH_FS_DYNAMIC_FILE>(handle);

	file_handles[file.get()] = file;

	return file.get();
}

int SimpleFlashFs_dynamic_fclose( SIMPLE_FLASH_FS_DYNAMIC_FILE* file_ptr )
{
	auto file_it = file_handles.find( file_ptr );

	if( file_it == file_handles.end() ) {
		return SIMPLE_FLASH_FS_DYNAMIC_EOF;
	}

	if( !file_ptr->handle->flush() ) {
		return SIMPLE_FLASH_FS_DYNAMIC_EOF;
	}

	file_handles.erase( file_it );

	return 0;
}

int SimpleFlashFs_dynamic_fflush( SIMPLE_FLASH_FS_DYNAMIC_FILE* file_ptr )
{
	if( !file_ptr->handle->flush() ) {
		return SIMPLE_FLASH_FS_DYNAMIC_EOF;
	}

	return 0;
}

size_t SimpleFlashFs_dynamic_fwrite(const void *ptr,
             size_t size, size_t nmemb,
             SIMPLE_FLASH_FS_DYNAMIC_FILE * stream)
{
	std::size_t ret = stream->handle->write( reinterpret_cast<const std::byte*>(ptr), size * nmemb );
	return ret / size;
}

size_t SimpleFlashFs_dynamic_fread(void *ptr,
             size_t size, size_t nmemb,
             SIMPLE_FLASH_FS_DYNAMIC_FILE * stream)
{
	std::size_t ret = stream->handle->read( reinterpret_cast<std::byte*>(ptr), size * nmemb );
	return ret / size;
}

int SimpleFlashFs_dynamic_feof( SIMPLE_FLASH_FS_DYNAMIC_FILE* stream )
{
	if( stream->handle->eof() ) {
		return SIMPLE_FLASH_FS_DYNAMIC_EOF;
	}

	return 0;
}

int SimpleFlashFs_dynamic_fgetc( SIMPLE_FLASH_FS_DYNAMIC_FILE *stream )
{
	std::byte b;
	if( stream->handle->read(&b,sizeof(b)) == sizeof(b) ) {
		return static_cast<int>(b);
	}

	return SIMPLE_FLASH_FS_DYNAMIC_EOF;
}

char *SimpleFlashFs_dynamic_fgets(char *s, int size, SIMPLE_FLASH_FS_DYNAMIC_FILE *stream)
{
	auto current_pos = stream->handle->tellg();
	auto len_read = stream->handle->read( reinterpret_cast<std::byte*>(s), size );

	if( len_read == 0 ) {
		return nullptr;
	}

	for( int i = 0; i < len_read; i++ ) {
		if( s[i] == '\n' ) {
			s[i+1] = '\0';
			stream->handle->seek(current_pos+i);
			return s;
		}
	}

	s[size] = '\0';
	return s;
}

int SimpleFlashFs_dynamic_fputc(int c, SIMPLE_FLASH_FS_DYNAMIC_FILE *stream)
{
	std::byte b = static_cast<std::byte>(c);
	if( stream->handle->write(&b,sizeof(b)) == sizeof(b) ) {
		return c;
	}

	return SIMPLE_FLASH_FS_DYNAMIC_EOF;
}

int SimpleFlashFs_dynamic_fputs(const char *s, SIMPLE_FLASH_FS_DYNAMIC_FILE *stream)
{
	std::size_t len = std::strlen(s);

	if( len == 0 ) {
		return 0;
	}

	const std::byte *b = reinterpret_cast<const std::byte*>(s);

	if( std::size_t len_written; (len_written = stream->handle->write(b,len)) > 0 ) {
		return len_written;
	}

	return SIMPLE_FLASH_FS_DYNAMIC_EOF;
}
