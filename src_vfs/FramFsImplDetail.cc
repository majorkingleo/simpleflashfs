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

std::optional<uint32_t> FramFsImplDetail::allocate_free_data_page()
{
	if( free_data_pages.empty() ) {
		CPPDEBUG( "no free data pages left" );
		return {};
	}

	std::uniform_int_distribution<std::mt19937::result_type> dist(0,free_data_pages.size()-1);
	int target_index = dist(m_rng);

	for( auto it = free_data_pages.begin(); it != free_data_pages.end(); ++it, --target_index )
	{
		if( target_index == 0 ) {
			uint32_t ret = *it;
			free_data_pages.erase(it);
			return ret;
		}
	}
	
	return {};
}

bool FramFsImplDetail::init() 
{
	if( !base_t::init() ) {
		m_initialized = false;
		return false;
	}

	m_header_inode_range.reinit( header );
	header_inode_range = &m_header_inode_range;

	m_initialized = true;
	return true;
}