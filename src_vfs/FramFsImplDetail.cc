#include "FramFsImplDetail.h"
#include <static_format.h>

using namespace Tools;

::SimpleFlashFs::Vfs::file_handle_t FramFsImplDetail::open( const std::string_view & path, std::ios_base::openmode mode )
{
	auto fh = base_t::open( path, mode );

	if( !fh ) {
		return nullptr;
	}

	return std::make_unique<FileHandle>( std::move(fh) );
}

bool FramFsImplDetail::list_files( std::function<bool(const std::string_view &, std::size_t size )> callback )
{
	for( auto inode : get_all_inodes() ) {
		if( !inode ) {
			continue;
		}

		// deleted file, ignore
		if( inode->inode.file_name.empty() ) {
			continue;
		}

		if( !callback( inode->inode.file_name, inode->file_size() ) ) {
			return true;
		}
	}

	return true;
}

void FramFsImplDetail::cleanup()
{
	for( auto inode : get_all_inodes() ) {
		if( !inode ) {
			continue;
		}
		
		if( inode->inode.file_name.empty() ) {
			mem->erase( header.page_size + inode->page * header.page_size, header.page_size );
		}
	}
}