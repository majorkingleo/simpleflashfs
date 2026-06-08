#pragma once

#include <SimpleFlashFsDynamic.h>
#include "SimpleFlashFsVfs.h"
#include <functional>

class FramFsImplDetail : public ::SimpleFlashFs::dynamic::SimpleFlashFs, public SimpleFlashFs::Vfs::VfsDriveInterface
{
public:
	using base_t = ::SimpleFlashFs::dynamic::SimpleFlashFs;

	struct Stat
	{
		std::size_t largest_file_size = 0; // in bytes
		std::size_t trash_size = 0; // in bytes
		std::size_t used_inodes = 0; // number of inodes in use
		std::size_t trash_inodes = 0; // number of inodes to delete
		std::size_t free_inodes = 0;
	};

private:

	class Trash
	{
		using inode_t = decltype(FramFsImplDetail::Inode::inode_number);
		using inode_version_t = decltype(FramFsImplDetail::Inode::inode_version_number);

	public:
		class Entry
		{
		public:
			struct data_t
			{
				inode_version_t 			inode_version_number;
				std::optional<uint32_t> 	page;

				bool operator<( const data_t & other ) const {
					return inode_version_number < other.inode_version_number;
				}
			};

			using vector_t = std::vector<data_t>;

		protected:

			vector_t 		inode_versions;
			inode_version_t max_version = 0;
			inode_t 		inode_number = 0;

		public:
			Entry( inode_t inode_number_ )
			: inode_number( inode_number_ )
			{
			}

			void add( inode_version_t version, std::optional<uint32_t> page );

			const vector_t & get_inode_versions_to_delete()
			{
				std::sort( inode_versions.begin(), inode_versions.end() );
				return inode_versions;
			}

			inode_version_t get_max_version() const {
				return max_version;
			}

			inode_t get_inode_number() const {
				return inode_number;
			}
		};

		using trash_inodes_t = std::vector<Entry>;

	private:
		trash_inodes_t trash_inodes;

	public:

		Entry & add( inode_t inode_number, inode_version_t version, std::optional<uint32_t> page );

		trash_inodes_t & get_trash_inodes() {
			return trash_inodes;
		}
	};

protected:
	Stat stat{};
	std::string m_drive_name;

public:
	FramFsImplDetail( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_, const std::string_view & drive_name_ )
	: SimpleFlashFs( mem_interface_ ), m_drive_name( drive_name_ )
	{}

	const Stat & get_stat() const {
		return stat;
	}

	// read the fs the memory interface points to
	// starting at offset 0
	bool init() {
		if( !base_t::init() ) {
			return false;
		}

		read_all_free_data_pages();
		return true;
	}

	// override VfsDriveInterface::open
	::SimpleFlashFs::Vfs::file_handle_t open( const std::string_view & path, std::ios_base::openmode mode ) override;

	bool list_files( std::function<bool(const std::string_view &, std::size_t size )> callback ) override;

	std::string_view get_drive_name() const override {
		return m_drive_name;
	}

private:
	// bring base_t::open into scope (hides the VfsDriveInterface version for direct calls)
	using base_t::open;

protected:
	void read_all_free_data_pages();
	bool is_empty( auto it_begin, auto it_end ) const;
	std::optional<FileHandle> read_inode( std::size_t index );
	void cleanup( Trash & trash );
};
