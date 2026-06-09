#include "FramFsImplDetail.h"
#include <static_format.h>

using namespace Tools;

// #define DO_DEBUG


void FramFsImplDetail::Trash::Entry::add( inode_version_t version, std::optional<uint32_t> page )
{
	vector_t::iterator it = std::find_if( inode_versions.begin(), inode_versions.end(), [version]( const data_t & data ) {
		return data.inode_version_number == version;
	} );

	if( it == inode_versions.end() ) {
		inode_versions.emplace_back( version, page );
		max_version = std::max( max_version, version );
	}
}

FramFsImplDetail::Trash::Entry & FramFsImplDetail::Trash::add( inode_t inode_number, inode_version_t version, std::optional<uint32_t> page )
{
	//CPPDEBUG( static_format<100>( "Adding: %d,%d at page: %d", inode_number, version, page.value_or(0) ) );

	for( auto & entry : trash_inodes ) {
		if( entry.get_inode_number() == inode_number ) {
			// CPPDEBUG( "here");
			entry.add( version, page );
			return entry;
		}
	}

	Entry & trash = trash_inodes.emplace_back( inode_number );
	trash.add( version, page );
	return trash;
}

/**
 * reads inode, does no error correction, don't use the resulting
 * file handle for reading, or writing.
 *
 * returns:
 *    empty optional:      read error occurred
 *    empty file handle:   crc error occurred, free page
 *    a valid file handle: inode is used by a file
 */
std::optional<typename FramFsImplDetail::FileHandle> FramFsImplDetail::read_inode( std::size_t index )
{
	typename base_t::config_t::page_type page( this->header.page_size );

	typename base_t::ReadPageReturn ret = base_t::read_page( index, page, true );
	if( !ret ) {
		if( ret.error == base_t::ReadError::ReadError ) {
#ifdef DO_DEBUG
				CPPDEBUG("read_error");
#endif
			return {};
		}

		if( ret.error == base_t::ReadError::CrcError ) {
			if( is_empty( page.begin(), page.end() ) ) {
				return FileHandle{};
			}
		}

		return {};
	}
	return this->get_inode( page, false );
}

bool FramFsImplDetail::is_empty( auto it_begin, auto it_end ) const
{
	const auto it_ret = std::find_if_not( it_begin, it_end, []( auto b ) { return b == std::byte(0xFF); } );

	if( it_ret == it_end ) {
		return true;
	}

	return false;
}
/*
void FramFsImplDetail::read_all_free_data_pages()
{
	free_data_pages.clear();
	m_stat = {};

	for( unsigned i = base_t::header.max_inodes;
		i < base_t::header.filesystem_size - 1; // -1 ... one page for the header itself
		i++ ) {

		free_data_pages.unordered_insert(i);
	}

	iv_storage.clear();

	Trash trash;

	for( unsigned i = 0; i < base_t::header.max_inodes; i++ ) {

		auto ret = read_inode( i );

		if( !ret ) {
			// empty optional, read error, remove from free data pages list
#ifdef DO_DEBUG
				CPPDEBUG( Tools::static_format<100>( "read error at page: %d", i ) );
#endif
			// erase page here?
			free_data_pages.erase(i);
			continue;
		}

		FileHandle & inode = *ret;

		if( !inode ) {
#ifdef DO_DEBUG
			if( i < 10) {
				CPPDEBUG( Tools::static_format<100>( "no inode at page: %d", i ) );
			}
#endif
			// free data page, do nothing it is already in the free_data_pages list
			continue;
		}

		inode.page = i;
		max_inode_number = std::max( max_inode_number, inode.inode.inode_number );

#ifdef DO_DEBUG
		{

			Tools::static_string<100> s_data_pages;

			for( const auto & page : inode.inode.data_pages ) {
				s_data_pages += static_format<100>( "%d, ", page.page_id ).c_str();
			}

			CPPDEBUG( Tools::static_format<100>( "found inode %d,%d at page: %d name: '%s' data pages: %d",
					inode.inode.inode_number, inode.inode.inode_version_number, i, inode.inode.file_name, s_data_pages.c_str() ) );


		}
#endif

		InodeVersionStore::cont_t previous_version;

		if( iv_storage.add( inode, previous_version ) == InodeVersionStore::add_ret_t::replaced ) {
			Trash::Entry & entry = trash.add( previous_version.first, previous_version.second, {} );
			entry.add( inode.inode.inode_version_number, inode.page );

			m_stat.trash_inodes++;
			m_stat.trash_size += header.page_size * inode.inode.data_pages.size();


		} else if( inode.inode.file_name.empty() ) {
			// deleted file with only one version
			trash.add( inode.inode.inode_number, inode.inode.inode_version_number, inode.page );

			m_stat.trash_inodes++;
			m_stat.trash_size += header.page_size * inode.inode.data_pages.size();

		} else {
			m_stat.used_inodes++;
		}

		m_stat.largest_file_size = std::max( m_stat.largest_file_size, inode.file_size() );

		// remove used pages from free_data_pages list
		for( auto page : inode.inode.data_pages ) {
			free_data_pages.erase(page.page_id);
		}
	}

	// CPPDEBUG( Tools::format( "max_inodes: %d", base_t::header.max_inodes ) );

	m_stat.free_inodes = header.max_inodes - m_stat.used_inodes - m_stat.trash_inodes;

#ifdef DO_DEBUG
	// CPPDEBUG( Tools::format( "free Data pages: %s", Tools::IterableToCommaSeparatedString(base_t::free_data_pages.get_data()) ) );
	CPPDEBUG( static_format<100>( "free Data pages: %d", base_t::free_data_pages.size() ) );
	CPPDEBUG( static_format<100>( "largest file size: %dB", m_stat.largest_file_size ) );
	CPPDEBUG( static_format<100>( "trash size size: %dB", m_stat.trash_size ) );
	CPPDEBUG( static_format<100>( "used inodes: %d trash inodes: %d free inodes: %d",
			m_stat.used_inodes,
			m_stat.trash_inodes,
			m_stat.free_inodes ));
#endif
	// cleanup trash
	cleanup( trash );
}
*/
void FramFsImplDetail::cleanup( Trash & trash )
{
	auto find_inode = [this]( inode_number_t inode_number, inode_version_number_t inode_version_number, std::optional<uint32_t> at_page ) -> std::optional<FileHandle>
	{
		for( unsigned i = at_page.value_or(0); i < base_t::header.max_inodes; i++ ) {

			typename config_t::page_type page(header.page_size);

			if( read_page( i, page, true ) ) {
				auto file_handle = get_inode( page, false );

				if( file_handle.inode.inode_number != inode_number ) {
					continue;
				}

				if( file_handle.inode.inode_version_number != inode_version_number ) {
					continue;
				}

				file_handle.page = i;
				return file_handle;
			}

		}

		return {};
	};


	for( Trash::Entry & te : trash.get_trash_inodes() ) {
		const Trash::Entry::vector_t & v = te.get_inode_versions_to_delete();
		if( v.size() < 2 ) {
			// CPPDEBUG( static_format<100>("invalid number of inode versions to delete, ignore inode: %d size: %d", te.get_inode_number(), v.size() ) );
			continue;
		}

		for( unsigned i = 0; i + 1 < v.size(); ++i ) {
			const Trash::Entry::data_t & data_older_inode = v[i];
			const Trash::Entry::data_t & data_newer_inode = v[i+1];

			auto o_older_inode = find_inode( te.get_inode_number(), data_older_inode.inode_version_number, data_older_inode.page );
			auto o_newer_inode = find_inode( te.get_inode_number(), data_newer_inode.inode_version_number, data_newer_inode.page );

			if( !o_older_inode ) {
				CPPDEBUG( static_format<100>("cannot find older inode: %d,%d", te.get_inode_number(), data_older_inode.inode_version_number ) );
				continue;
			}

			if( !o_newer_inode ) {
				CPPDEBUG( static_format<100>("cannot find newer inode: %d,%d", te.get_inode_number(), data_newer_inode.inode_version_number ) );
				continue;
			}

			file_handle_t & older_inode = *o_older_inode;
			file_handle_t & newer_inode = *o_newer_inode;

#ifdef DO_DEBUG
			CPPDEBUG( static_format<100>("erasing inode: %d,%d at page: %d data pages %d",
					older_inode.inode.inode_number,
					older_inode.inode.inode_version_number,
					older_inode.page,
					older_inode.inode.data_pages.size() ) );
#endif
			/*
			auto to_int_list = []( const auto & vec ) {
				std::vector<uint32_t> ret;
				for( auto & v : vec ) {
					ret.push_back( v.page_id );
				}
				return ret;
			};


			CPPDEBUG( Tools::format( "Data pages old: %s", Tools::IterableToCommaSeparatedString(to_int_list(older_inode.inode.data_pages)) ) );
			CPPDEBUG( Tools::format( "Data pages new: %s", Tools::IterableToCommaSeparatedString(to_int_list(newer_inode.inode.data_pages)) ) );
			*/
			erase_inode_and_unused_pages( older_inode, newer_inode );

		}
	}

	// delete single entries, with empty file name. These are deleted files
	for( Trash::Entry & te : trash.get_trash_inodes() ) {
		const Trash::Entry::vector_t & v = te.get_inode_versions_to_delete();
		if( v.size() != 1 ) {
			continue;
		}
		const Trash::Entry::data_t & data_inode = v[0];

		auto o_inode = find_inode( te.get_inode_number(), data_inode.inode_version_number, data_inode.page );

		if( !o_inode ) {
			CPPDEBUG( static_format<100>("cannot find inode: %d,%d", te.get_inode_number(), data_inode.inode_version_number ) );
			continue;
		}

		file_handle_t & inode = *o_inode;

#ifdef DO_DEBUG
		CPPDEBUG( static_format<100>("erasing inode: %d,%d at page: %d data pages %d",
				inode.inode.inode_number,
				inode.inode.inode_version_number,
				inode.page,
				inode.inode.data_pages.size() ) );
#endif

		std::size_t address = header.page_size + inode.page * header.page_size;
		mem->erase(address, header.page_size );

		if( inode.page > header.max_inodes ) {
			free_data_pages.insert(inode.page);
		}

	} // for
}


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

		if( !callback( inode->inode.file_name, inode->file_size() ) ) {
			return true;
		}
	}

	return true;
}