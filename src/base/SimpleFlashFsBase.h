/**
 * SimpleFlashFs  base class
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */

#pragma once

#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include "../SimpleFlashFsConstants.h"
#include "../SimpleFlashFsFlashMemoryInterface.h"
#include "../SimpleFlashFsFileInterface.h"
#include "SimpleFlashFsPageSet.h"
#include <CpputilsDebug.h>
#include <static_format.h>
#include <string_utils.h>
#include <bit>
#include <span>
#include <optional>

namespace SimpleFlashFs {

class FlashMemoryInterface;

namespace base {

// https://stackoverflow.com/a/35092546
/**
 * swaps the byte order of a value
 */
template<typename T> inline static T swapByteOrder(const T& val) {
    int totalBytes = sizeof(val);
    T swapped = (T) 0;
    for (int i = 0; i < totalBytes; ++i) {
        swapped |= (val >> (8*(totalBytes-i-1)) & 0xFF) << (8*i);
    }
    return swapped;
}

/**
 * Filesystem header
 */
template <class Config>
struct Header
{
	enum class ENDIANNESS
	{
		LE,
		BE
	};

	enum class CRC_CHECKSUM
	{
		CRC32 = 0
	};

	Config::magic_string_type 	magic_string;
	ENDIANNESS 					endianness{ENDIANNESS::LE};
	uint16_t   					version = 0;
	uint32_t					page_size = 0;
	uint64_t					filesystem_size = 0; // size in pages
	uint32_t					max_inodes = 0;
	uint16_t					max_path_len = 0;
	CRC_CHECKSUM				crc_checksum_type{CRC_CHECKSUM::CRC32};
};

enum class InodeAttribute
{
	NONE    = 0,
	SPECIAL = 1,
};

/**
 * Inode struct
 */
template <class Config>
struct Inode
{
	uint64_t 				inode_number{};
	uint64_t				inode_version_number{};
	uint16_t				file_name_len{};
	Config::string_type		file_name;
	uint64_t				attributes{};
	uint64_t				file_len{};
	uint32_t				pages{};

	using data_pages_value_type = uint32_t;
	static constexpr uint32_t data_pages_type_size = sizeof(data_pages_value_type);

	struct data_page_t {
		enum class State
		{
			New,
			Stored,
			Deleted
		};

		data_pages_value_type page_id;
		State                 state = State::Stored;
	};

	// the index of data pages
	typename Config::template vector_type<data_page_t> data_pages;

	// data, that can be stored inside the inode
	// will only be filled if pages == 0, so data_pages is also
	// zero and needs no space
	Config::page_type inode_data;

	std::size_t count_valid_data_pages() const {
		return std::count_if( data_pages.begin(), data_pages.end(), []( auto & p ) {
			return p.state != data_page_t::State::Deleted;
		});
	}
};

/**
 * FileHandle class
 */
template <class Config,class FS>
class FileHandle : public FileInterface
{
public:
	base::Inode<Config> inode {};
	uint32_t page   {0};
	std::size_t pos {0};
	bool modified   {false};
	bool append     {false}; // always write at the end of file

protected:
	FS *fs;

public:
	FileHandle( FS *fs_ = nullptr )
	: fs( fs_ )
	{}

	FileHandle( FileHandle && other )
	: inode( other.inode ),
	  page( other.page ),
	  pos( other.pos ),
	  modified( other.modified ),
	  append( other.append ),
	  fs( other.fs )
	{
		other.fs = nullptr;
	}

	FileHandle( const FileHandle & other ) = delete;

	/**
	 * automatically calls flush()
	 */
	~FileHandle()
	{
		if( fs ) {
			flush();

			fs->free_unwritten_pages( page );
			for( auto page : inode.data_pages ) {
				fs->free_unwritten_pages(page.page_id);
			}
		}
	}


	FileHandle & operator=( const FileHandle & other ) = delete;

	FileHandle & operator=( FileHandle && other ) {
		inode = other.inode;
		page = other.page;
		pos = other.pos;
		modified = other.modified;
		append = other.append;
		fs = other.fs;

		other.fs = nullptr;

		return *this;
	}

	bool operator!() const override {
		return (fs == nullptr);
	}


	std::size_t write( const std::byte *data, std::size_t size ) override {
		if( append ) {
			seek( inode.file_len );
		}
		return fs->write( this, data, size );
	}

	std::size_t read( std::byte *data, std::size_t size ) override {
		return fs->read( this, data, size );
	}

	bool flush() override {
		return fs->flush(this);
	}

	std::size_t tellg() const override {
		return pos;
	}

	std::size_t file_size() const override {
		return inode.file_len;
	}

	bool eof() const override {
		if( inode.file_len == 0 ) {
			return true;
		}

		return pos == inode.file_len-1;
	}

	bool seek( std::size_t pos_ ) override {

		if( pos_ < inode.file_len || pos_ == inode.file_len ) {
			pos = pos_;
			return true;
		}

		// enlarge file
		if( !fs->enlarge_file( this, pos_ - inode.file_len -1 ) ) {
			return false;
		}

		pos = pos_;
		return true;
	}

	FileHandle get_disconnected_copy() const {
		FileHandle ret{};

		ret.inode = inode;
		ret.page = page;
		ret.pos = pos;
		ret.modified = modified;
		ret.append = append;

		return ret;
	}

	bool delete_file() override {
		return fs->delete_file( this );
	}

	bool rename_file( const std::string_view & new_file_name ) override {
		return fs->rename_file( this, new_file_name );
	}

	void disconnect() {
		fs = nullptr;
	}

	bool valid() const {
		return fs != nullptr;
	}

	std::string_view get_file_name() const override {
		return inode.file_name;
	}

	bool is_append_mode() const override {
		return append;
	}
};


template <class Config>
class SimpleFlashFsBase
{
public:
	using header_t = Header<Config>;
	using inode_t = Inode<Config>;
	using file_handle_t = FileHandle<Config,SimpleFlashFsBase<Config>>;
	using config_t = Config;
	using inode_number_t  = decltype(Inode<Config>::inode_version_number);
	using inode_version_number_t = decltype(Inode<Config>::inode_version_number);

protected:
	using data_page_t = Inode<Config>::data_page_t;

	class InodeVersionStore
	{
	public:

		enum class add_ret_t
		{
			replaced,
			inserted
		};

		struct InodeVersion
		{
			inode_number_t inode = 0;
			uint32_t page = 0;
			inode_version_number_t version = 0;

			InodeVersion() = default;
			InodeVersion( const file_handle_t & file )
			: inode( file.inode.inode_number ),
			  page( file.page ),
			  version( file.inode.inode_version_number )
			{

			}

			InodeVersion & operator=( const file_handle_t & file )
			{
				inode = file.inode.inode_number;
				page = file.page;
				version = file.inode.inode_version_number;
				return *this;
			}
		};

	protected:
		typename Config::template vector_type<InodeVersion> data;

	public:

		using cont_t = std::pair<inode_number_t,inode_version_number_t>;

		add_ret_t add( const file_handle_t & file, cont_t & previous_data )
		{
			for( auto & iv : data ) {
				if( iv.inode == file.inode.inode_number ) {
					previous_data.first = iv.inode;
					previous_data.second = iv.version;

					if( file.inode.inode_version_number > iv.version ) {
						iv.version = file.inode.inode_version_number;
						iv.page = file.page;
					}
					return add_ret_t::replaced;
				}
			}

			data.emplace_back( file );
			return add_ret_t::inserted;
		}

		add_ret_t add( const file_handle_t & file )
		{
			cont_t dummy;
			return add( file, dummy );
		}

		const typename Config::template vector_type<InodeVersion> & get_data() const {
			return data;
		}

		void clear() {
			data.clear();
		}
	};

	enum class ReadError
	{
		ReadError,
		CrcError
	};

	struct ReadPageReturn
	{
		std::optional<ReadError> error;

		bool operator!() const {
			return error.has_value();
		}

		operator bool() const {
			return !error.has_value();
		}
	};

	struct ReadPageMappedReturn
	{
		std::optional<ReadError> error;
		std::optional<std::span<const std::byte>> data;

		ReadPageMappedReturn( ReadError error_ )
		: error( error_ )
		{}

		ReadPageMappedReturn( std::span<const std::byte> data_ )
		: data( data_ )
		{}

		bool operator!() const {
			return error.has_value();
		}

		operator bool() const {
			return !error.has_value();
		}
	};

protected:
	header_t header {};
	FlashMemoryInterface *mem;

	// to be replaced by a static version later
	PageSet<Config> allocated_unwritten_pages;
	PageSet<Config> free_data_pages;

	uint64_t max_inode_number = 0;

	// Buffer of a member variable here to avoid having it on the
	// stack of the function, because it can be really large.
	InodeVersionStore iv_storage;

public:
	SimpleFlashFsBase( FlashMemoryInterface *mem_interface_ )
	: mem(mem_interface_)
	{

	}

	virtual ~SimpleFlashFsBase() {}

	Header<Config> create_default_header( uint32_t page_size, uint64_t filesystem_size_in_pages );

	const Header<Config> & get_header() const {
		return header;
	}

	file_handle_t open( const Config::string_view_type & name, std::ios_base::openmode mode );

	std::size_t write( file_handle_t* file, const std::byte *data, std::size_t size );

	std::size_t read( file_handle_t* file, std::byte *data, std::size_t size );

	bool flush( file_handle_t* file );

	bool delete_file( file_handle_t* file );

	bool rename_file( file_handle_t* file, const std::string_view & new_file_name );

	bool enlarge_file( file_handle_t* file, std::size_t amount );

	uint64_t get_max_inode_number() const {
		return max_inode_number;
	}

	// it is a very rare use case to set this from outside.
	// don't to it
	void set_max_inode_number( uint64_t max_inode_number_ ) {
		max_inode_number = max_inode_number_;
	}

	std::size_t get_number_of_free_data_pages() const {
		return free_data_pages.size();
	}

	/**
	 * returns a string_view to the filename. This is only possible if the flash
	 * data is mapped to RAM as address. This is the case for the internal flash.
	 */
	std::optional<typename Config::string_view_type> get_inode_file_name_mapped( const file_handle_t & file_handle ) const;

	friend class FileHandle<Config,SimpleFlashFsBase<Config>>;

	// read the fs that the memory interface points to
	// starting at offset 0
	virtual bool init();

	std::size_t get_max_file_size() const {
		 return (header.page_size * (header.filesystem_size - 1)) - (header.max_inodes * header.page_size);
	}

protected:
	bool swap_endianess() const;

	template<class T> void auto_endianess( T & val ) const {
		if( swap_endianess() ) {
			val = swapByteOrder( val );
		}
	}

	std::size_t get_num_of_checksum_bytes() const;

	bool write( const Header<Config> & header );

	void add_page_checksum( Config::page_type & page );

	uint32_t calc_page_checksum( const Config::page_type & page ) {
		return calc_page_checksum( page.data(), page.size() );
	}
	uint32_t calc_page_checksum( const std::byte *page, std::size_t size );

	uint32_t get_page_checksum( const Config::page_type & page ) {
		return get_page_checksum( page.data(), page.size() );
	}
	uint32_t get_page_checksum( const std::byte *page, std::size_t size );

	uint32_t get_checksum_size() const;

	ReadPageReturn read_page( std::size_t idx, Config::page_type & page, bool check_crc = false ) {
		return read_page( idx, page.data(), page.size(), check_crc );
	}

	ReadPageReturn read_page( std::size_t idx, std::byte *data, std::size_t size, bool check_crc = false );

	ReadPageMappedReturn read_page_mapped( std::size_t idx, std::size_t size, bool check_crc = false );

	Config::page_type inode2page( const Inode<Config> & inode );

	file_handle_t find_file( const Config::string_view_type & name ) {
		if( mem->can_map_read() ) {
			return find_file_mapped(name);
		}
		return find_file_unmapped(name);
	}

	file_handle_t find_file_mapped( const Config::string_view_type & name );
	file_handle_t find_file_unmapped( const Config::string_view_type & name );

	file_handle_t get_inode( const Config::page_type & data, bool do_error_corrections = true ) {
		return get_inode( std::span<const std::byte>(data.data(),data.size()), do_error_corrections );
	}

	file_handle_t get_inode( const std::span<const std::byte> & data, bool do_error_corrections = true );

	file_handle_t allocate_free_inode_page() {
		if( mem->can_map_read() ) {
			return allocate_free_inode_page_mapped();
		}
		return allocate_free_inode_page_unmapped();
	}

	file_handle_t allocate_free_inode_page_mapped();
	file_handle_t allocate_free_inode_page_unmapped();

	void free_unwritten_pages( uint32_t page ) {
		allocated_unwritten_pages.erase(page);
	}

	uint32_t allocate_free_inode_page_number() {
		if( mem->can_map_read() ) {
			return allocate_free_inode_page_number_mapped();
		}
		return allocate_free_inode_page_number_unmapped();
	}

	uint32_t allocate_free_inode_page_number_mapped();
	uint32_t allocate_free_inode_page_number_unmapped();

	uint32_t allocate_free_data_page();
	uint32_t allocate_free_data_page( const file_handle_t *file );

	/**
	 * returns the maximum size of data, that fits inside an spacific inode
	 * depends, on the filename len
	 */
	std::size_t get_inode_data_space_size( const file_handle_t* file ) const;

	/**
	 * The length of the filename is dynamic, so we have to calculate
	 * it per file.
	 */
	std::size_t get_max_inode_data_pages( const file_handle_t* file ) const;

	virtual void erase_inode_and_unused_pages( file_handle_t & inode_to_erase, file_handle_t & next_inode_version );


	bool write_page( file_handle_t* file,
			const std::basic_string_view<std::byte> & page,
			typename Inode<Config>::data_page_t & page_meta );

	bool write_page( file_handle_t* file,
			const Config::page_type & page,
			typename Inode<Config>::data_page_t & page_meta ) {

		return write_page( file,
				std::basic_string_view<std::byte>( page.data(), page.size() ),
				page_meta );
	}

	bool write_meta_page( file_handle_t* file,
			const Config::page_type & page,
			 uint32_t idx ) {

		data_page_t page_meta { idx, data_page_t::State::New };

		return write_page( file,
				std::basic_string_view<std::byte>( page.data(), page.size() ),
				page_meta );
	}

	bool allocate_new_data_pages( std::size_t page_idx, file_handle_t* file );

	/**
	 * replaces the existing page at data_pages with a new allocated page
	 */
	bool allocate_new_data_page_at( std::size_t page_idx, file_handle_t* file );

	/***
	 * writes unwritten pages, filled with zeros
	 * This can happen when seeking to > file size position in the file.
	 */
	bool write_zero_pages( file_handle_t* file );
};

template <class Config>
Header<Config> SimpleFlashFsBase<Config>::create_default_header( uint32_t page_size, uint64_t filesystem_size )
{
	header_t header;

	header.magic_string.assign(MAGICK_STRING);
	header.version = 1;
	header.page_size = page_size;
	header.filesystem_size = filesystem_size;

	header.max_inodes = filesystem_size / 10 * 2;
	header.max_inodes = std::max( header.max_inodes, static_cast<uint32_t>(10) );

	if( header.max_inodes > header.filesystem_size ) {
		header.max_inodes = 2;
	}

	header.max_path_len = 50;

	if( std::endian::native == std::endian::big ) {
		header.endianness = header_t::ENDIANNESS::BE;
	} else {
		header.endianness = header_t::ENDIANNESS::LE;
	}

	return header;
}

template <class Config>
bool SimpleFlashFsBase<Config>::swap_endianess() const
{
	if( std::endian::native == std::endian::big && header.endianness == header_t::ENDIANNESS::LE ) {
		return true;
	}
	else if( std::endian::native == std::endian::little && header.endianness == header_t::ENDIANNESS::BE ) {
		return true;
	}

	return false;
}

template <class Config>
std::size_t SimpleFlashFsBase<Config>::get_num_of_checksum_bytes() const
{
	switch( header.crc_checksum_type )
	{
	case header_t::CRC_CHECKSUM::CRC32: return sizeof(uint32_t);

	default:
		throw std::out_of_range("unknown checksum type");
	}
}

template <class Config>
bool SimpleFlashFsBase<Config>::write( const Header<Config> & header_ )
{
	header = header_;
	Header h = header_;

	typename Config::page_type page(header.page_size);

	std::size_t pos = 0;

	std::memcpy( &page[pos], header.magic_string.c_str(), header.magic_string.size() );
	pos += MAGICK_STRING_LEN;

	if( header.endianness == header_t::ENDIANNESS::LE ) {
		std::memcpy( &page[pos], ENDIANNESS_LE, 2 );
	} else {
		std::memcpy( &page[pos], ENDIANNESS_BE, 2 );
	}

	pos += 2;

	auto add=[this,&pos,&page]( auto & t ) {
		auto_endianess(t);
		const size_t size = sizeof(std::remove_reference_t<decltype(t)>);
		std::memcpy( &page[pos], &t, size );
		pos += size;
	};

	add(h.version);
	add(h.page_size);
	add(h.filesystem_size);
	add(h.max_inodes);
	add(h.max_path_len);
	uint16_t chktype = static_cast<uint16_t>(h.crc_checksum_type);
	add(chktype);


	add_page_checksum( page );

	mem->write( 0, page.data(), page.size() );

	return true;
}

template <class Config>
void  SimpleFlashFsBase<Config>::add_page_checksum( Config::page_type & page )
{
	switch( header.crc_checksum_type )
	{
	case header_t::CRC_CHECKSUM::CRC32:
		{
			uint32_t chksum = Config::crc32( &page[0], page.size() - sizeof(uint32_t));
			auto_endianess(chksum);
			memcpy( &page[page.size() - sizeof(uint32_t)], &chksum, sizeof(chksum));
			return;
		}
	default:
		throw std::out_of_range("unknown checksum type");
	}
}

template <class Config>
uint32_t SimpleFlashFsBase<Config>::calc_page_checksum( const std::byte *page, std::size_t size )
{
	switch( header.crc_checksum_type )
	{
	case header_t::CRC_CHECKSUM::CRC32:
		return Config::crc32( page, size - sizeof(uint32_t));
	default:
		throw std::out_of_range("unknown checksum type");
	}
}

template <class Config>
uint32_t SimpleFlashFsBase<Config>::get_page_checksum( const std::byte *page, std::size_t size )
{
	switch( header.crc_checksum_type )
	{
	case header_t::CRC_CHECKSUM::CRC32:
		{
			uint32_t chksum = 0;
			std::memcpy( &chksum, page + (size - sizeof(uint32_t)), sizeof(uint32_t) );
			auto_endianess(chksum);
			return chksum;
		}
	default:
		throw std::out_of_range("unknown checksum type");
	}
}

template <class Config>
uint32_t SimpleFlashFsBase<Config>::get_checksum_size() const
{
	switch( header.crc_checksum_type )
	{
	case header_t::CRC_CHECKSUM::CRC32:
		{
			return sizeof(uint32_t);
		}
	default:
		throw std::out_of_range("unknown checksum type");
	}
}

template <class Config>
SimpleFlashFsBase<Config>::ReadPageReturn SimpleFlashFsBase<Config>::read_page( std::size_t idx, std::byte *page, std::size_t size, bool check_crc )
{
	std::size_t offset = header.page_size + idx * header.page_size;
	if( std::size_t len_read; (len_read = mem->read(offset, page, size )) != size ) {
		CPPDEBUG( "cannot read all data" );
		//CPPDEBUG( Tools::static_format<100>( "cannot read all data from page: %d size: %d len_read: %d offset: %d", idx, size, len_read, offset ) );
		return { ReadError::ReadError };
	}

	if( check_crc ) {
		if( get_page_checksum( page, size ) != calc_page_checksum(page, size) ) {
			//CPPDEBUG( "checksum failed" );
			return { ReadError::CrcError };
		}
	}

	return {};
}

template <class Config>
SimpleFlashFsBase<Config>::ReadPageMappedReturn SimpleFlashFsBase<Config>::read_page_mapped( std::size_t idx, std::size_t size, bool check_crc )
{
	std::size_t offset = header.page_size + idx * header.page_size;

	const std::byte* addr = mem->map_read(offset, size );

	if( addr == nullptr ) {
		// CPPDEBUG( "cannot read all data" );
		// CPPDEBUG( Tools::static_format<100>("cannot read all data at page: %d", idx ) );
		return { ReadError::ReadError };
	}

	// pack data into the return value too, so additional verification might happens too
	SimpleFlashFsBase<Config>::ReadPageMappedReturn ret( { addr, size } );

	if( check_crc ) {
		if( get_page_checksum( addr, size ) != calc_page_checksum(addr, size) ) {
			/*
			if( idx < 10 ) {
				CPPDEBUG( Tools::static_format<100>("checksum failed at page: %d", idx ) );
			}*/
			ret.error = ReadError::CrcError;
		}
	}

	return ret;
}


template <class Config>
Config::page_type SimpleFlashFsBase<Config>::inode2page( const Inode<Config> & inode )
{
	typename Config::page_type page(header.page_size);
	std::size_t pos = 0;

	auto write=[this,&pos,&page]( auto t ) {
		auto_endianess(t);
		const size_t size = sizeof(t);

		if( pos + sizeof(size) > page.size() ) {
			//CPPDEBUG( Tools::static_format<100>( "%d + %d > %d", pos, sizeof(size), page.size() ) );
			//throw new std::out_of_range( Tools::static_format<100>( "%d + %d > %d", pos, sizeof(size), page.size() ));
			throw new std::out_of_range( "writing beyond page size" );
		}

		std::memcpy(&page[pos], &t, size );
		pos += sizeof(t);
	};

	write( inode.inode_number );
	write( inode.inode_version_number );
	write( inode.file_name_len );

	std::memcpy(&page[pos], &inode.file_name[0], inode.file_name_len );
	pos += inode.file_name_len;

	write( inode.attributes );
	write( inode.file_len );

	uint32_t pages = inode.data_pages.size();
	write( pages );

	for( unsigned i = 0; i < pages; i++ ) {
		write( inode.data_pages[i].page_id );
	}

	// write small data directly into the inode
	if( pages == 0 && inode.file_len > 0 ) {
		memcpy( page.data() + pos, inode.inode_data.data(), inode.inode_data.size() );
	}

	return page;
}

template <class Config>
bool SimpleFlashFsBase<Config>::init()
{
	header_t h{};
	typename Config::page_type page(MIN_PAGE_SIZE);

	mem->read(0, page.data(), page.size());

	std::size_t pos = 0;
	h.magic_string.assign(std::string_view( reinterpret_cast<char*>(&page[pos]), MAGICK_STRING_LEN ));

	if( h.magic_string != MAGICK_STRING ) {
		//CPPDEBUG( Tools::static_format<100>( "invalid magick string: '%s'", h.magic_string ));
		CPPDEBUG( "invalid magick string" );
		return false;
	}

	pos += MAGICK_STRING_LEN;

	std::string_view endianness( reinterpret_cast<char*>(&page[pos]), ENDIANNESS_LEN );

	if( endianness == ENDIANNESS_BE ) {
		h.endianness = header_t::ENDIANNESS::BE;
	} else if( endianness == ENDIANNESS_LE ) {
		h.endianness = header_t::ENDIANNESS::LE;
	} else {
		// CPPDEBUG( Tools::static_format<100>( "invalid endianness: '%s'", endianness ));
		CPPDEBUG( "invalid endianness" );
		return false;
	}

	// auto_endianess is looking at header.endianess
	header = h;

	pos += ENDIANNESS_LEN;

	auto read=[this,&pos,&page]( auto & t ) {
		const size_t size = sizeof(std::remove_reference_t<decltype(t)>);
		std::memcpy(&t, &page[pos], size );
		auto_endianess(t);
		pos += size;
	};

	read(h.version);
	read(h.page_size);
	read(h.filesystem_size);
	read(h.max_inodes);
	read(h.max_path_len);
	uint16_t chktype = 0;
	read(chktype);

	switch( chktype ) {
	case static_cast<uint16_t>(header_t::CRC_CHECKSUM::CRC32):
		h.crc_checksum_type = header_t::CRC_CHECKSUM::CRC32;
		break;
	default:
		//CPPDEBUG( Tools::static_format<100>( "invalid chksum type: '%d'", chktype ));
		CPPDEBUG( "invalid checksum type" );
		return false;
	}

	if( h.page_size < MIN_PAGE_SIZE ) {
		//CPPDEBUG( Tools::static_format<100>( "invalid page size '%d'", h.page_size ));
		CPPDEBUG( "page size lower mininum page size" );
		return false;
	}

	if( Config::PAGE_SIZE > 0 && h.page_size > Config::PAGE_SIZE ) {
		//CPPDEBUG( Tools::static_format<100>( "invalid page size '%d'", h.page_size ));
		CPPDEBUG( "page size larger than configured page size" );
		return false;
	}


	// now reread the whole page
	page.resize(h.page_size);
	mem->read(0, page.data(), page.size());

	uint32_t chksum_calc = calc_page_checksum( page );
	uint32_t chksum_page = get_page_checksum( page );

	if( chksum_calc != chksum_page ) {
		//CPPDEBUG( Tools::static_format<100>( "chksum_calc: %d != chksum_page: %d", chksum_calc, chksum_page ));
		CPPDEBUG( "checksum mismatch" );
		return false;
	}

	header = h;

	return true;
 }

template <class Config>
FileHandle<Config,SimpleFlashFsBase<Config>> SimpleFlashFsBase<Config>::open( const Config::string_view_type & name, std::ios_base::openmode mode )
{
	auto handle = find_file( name );

	if( !handle ) {
		// file does not exists
		if( !(mode & std::ios_base::out) && !(mode & std::ios_base::app)) {
			// CPPDEBUG( "no out mode" );
			return {};
		}

		handle = allocate_free_inode_page();

		if( !handle ) {
			CPPDEBUG( "cannot allocate new inode page" );
			return {};
		}

		handle.inode.file_name = name;
		handle.inode.file_name_len = name.size();
		handle.modified = true;

		if( mode & std::ios_base::app ) {
			handle.append = true;
		}

		if( mode & std::ios_base::ate ) {
			if( handle.inode.file_len > 0 ) {
				handle.pos = handle.inode.file_len - 1;
			}
		}

		return handle;
	}
	else if( mode & std::ios_base::trunc ) {

		auto new_handle = allocate_free_inode_page();

		if( !new_handle ) {
			CPPDEBUG( "cannot allocate new inode page" );
			return {};
		}

		new_handle.inode = handle.inode;
		new_handle.inode.file_len = 0;
		new_handle.inode.pages = 0;
		new_handle.inode.data_pages.clear();
		new_handle.inode.inode_data.clear();

		return new_handle;
	}

	return handle;
}

template <class Config>
FileHandle<Config,SimpleFlashFsBase<Config>> SimpleFlashFsBase<Config>::find_file_unmapped( const Config::string_view_type & name )
{
	iv_storage.clear();

	// find the latest version of all inodes
	// we have top do this, because only the last version of each
	// inode has it's last valid name
	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		typename Config::page_type page(header.page_size);

		if( read_page( i, page, true ) ) {
			auto file_handle = get_inode( page, false );
			file_handle.page = i;

			iv_storage.add(file_handle);
		}
	}

	for( auto & iv : iv_storage.get_data() ) {

		typename Config::page_type page(header.page_size);
		if( read_page( iv.page, page, true ) ) {
			auto file_handle = get_inode( page );
			file_handle.page = iv.page;

			// deleted file
			if( file_handle.inode.file_name.empty() ) {
				continue;
			}

			if( file_handle.inode.file_name == name ) {
				/*
				CPPDEBUG( Tools::format( "found file: '%s' Version: '%d' at page %d",
							name, file_handle.inode.inode_version_number, file_handle.page ));
							*/
				return file_handle;
			}
		}
	}

	return {};
}


template <class Config>
FileHandle<Config,SimpleFlashFsBase<Config>> SimpleFlashFsBase<Config>::find_file_mapped( const Config::string_view_type & name )
{
	iv_storage.clear();

	// find the latest version of all inodes
	// we have top do this, because only the last version of each
	// inode has it's last valid name
	for( unsigned i = 0; i < header.max_inodes; i++ ) {
		ReadPageMappedReturn ret = read_page_mapped( i, header.page_size, true );

		if( ret ) {
			auto page = *ret.data;
			auto file_handle = get_inode( page, false );
			file_handle.page = i;

			iv_storage.add(file_handle);
		}
	}

	for( auto & iv : iv_storage.get_data() ) {

		ReadPageMappedReturn ret = read_page_mapped( iv.page, header.page_size, true );

		if( ret ) {
			auto page = *ret.data;
			auto file_handle = get_inode( page );
			file_handle.page = iv.page;

			// deleted file
			if( file_handle.inode.file_name.empty() ) {
				continue;
			}

			if( file_handle.inode.file_name == name ) {
				return file_handle;
			}
		}
	}

	return {};
}

template <class Config>
std::optional<typename Config::string_view_type> SimpleFlashFsBase<Config>::get_inode_file_name_mapped( const file_handle_t & file_handle ) const
{
	if( !mem->can_map_read() ) {
		return {};
	}

	std::size_t offset = header.page_size + file_handle.page * header.page_size;

	const std::byte* addr = mem->map_read(offset, header.page_size );

	if( addr == nullptr ) {
		CPPDEBUG( "cannot read all data" );
		return {};
	}

	std::span<const std::byte> page( addr, header.page_size );

	std::size_t pos = sizeof( inode_t::inode_number) +
	   				  sizeof( inode_t::inode_version_number );

	decltype(inode_t::file_name_len) file_name_len = 0;

	auto read=[this,&pos,&page]( auto & t ) {
		const size_t size = sizeof(std::remove_reference_t<decltype(t)>);
		std::memcpy(&t, &page[pos], size );
		auto_endianess(t);
		pos += size;
	};

	read( file_name_len );

	typename Config::string_view_type ret = std::string_view( reinterpret_cast<const char*>(&page[pos]), file_name_len );

	return ret;
}

template <class Config>
FileHandle<Config,SimpleFlashFsBase<Config>> SimpleFlashFsBase<Config>::get_inode( const std::span<const std::byte> & page, bool do_error_corrections )
{
	file_handle_t ret(this);

	std::size_t pos = 0;

	auto read=[this,&pos,&page]( auto & t ) {
		const size_t size = sizeof(std::remove_reference_t<decltype(t)>);
		std::memcpy(&t, &page[pos], size );
		auto_endianess(t);
		pos += size;
	};

	read( ret.inode.inode_number );
	read( ret.inode.inode_version_number );
	read( ret.inode.file_name_len );

	ret.inode.file_name = std::string_view( reinterpret_cast<const char*>(&page[pos]), ret.inode.file_name_len );
	pos += ret.inode.file_name_len;

	read( ret.inode.attributes );
	read( ret.inode.file_len );
	read( ret.inode.pages );

	if( ret.inode.pages  ) {
		ret.inode.data_pages.reserve( ret.inode.pages );

		std::size_t len_read = 0;

		for( unsigned i = 0; i < ret.inode.pages; i++ ) {
			typename Inode<Config>::data_pages_value_type page_id{};

			read( page_id );

			if( do_error_corrections ) {
				if( page_id >= header.filesystem_size ) {

					int file_len = ret.inode.file_len;

					// is last page
					if( i + 1 ==  ret.inode.pages ) {

						int part_len = file_len % header.page_size;

						if( part_len == 0 ) {
							part_len = header.page_size;
						}
						file_len -= part_len;

					} else {
						file_len -= header.page_size;
					}

					/*
					CPPDEBUG( Tools::static_format<100>("file '%s' page_id %d > filesystem_size %d reducing file size from %d to %d",
							  ret.inode.file_name,
							  page_id,
							  header.filesystem_size,
							  ret.inode.file_len,
							  file_len < 0 ? 0 : file_len ) );
					*/
					if( file_len < 0 ) {
						ret.inode.file_len = 0;
					}

					ret.inode.file_len = file_len;
					ret.modified = true;

					continue;
				}


				if( len_read >= ret.inode.file_len ) {

					unsigned expected_pages = ret.inode.file_len / header.page_size;
					if( ret.inode.file_len % header.page_size ) {
						expected_pages++;
					}

					/*
					CPPDEBUG( Tools::static_format<100>("file '%s' %d,%d idx %d page_id %d > expected file size of %d pages, ignoring",
							  ret.inode.file_name,
							  ret.inode.inode_number,
							  ret.inode.inode_version_number,
							  i,
							  page_id,
							  expected_pages,
							  header.filesystem_size ) );
					*/
					ret.modified = true;
					break;
				}
			} // if( do_error_corrections )

			ret.inode.data_pages.push_back( { page_id } );
			len_read += header.page_size;
		}
	}
	/**
	 * read the data, that is directly stored inside the inode
	 */
	else if( ret.inode.pages == 0 && ret.inode.file_len > 0 ) {
		file_handle_t *handle = &ret;
		const std::size_t inode_space = get_inode_data_space_size(handle);

		if( ret.inode.file_len <= inode_space ) {
			ret.inode.inode_data.resize(inode_space);
			std::memcpy( ret.inode.inode_data.data(), page.data() + pos, ret.inode.file_len );
		} else {
			// CPPDEBUG( "inode with file length > inode space found, but no data pages" );
			ret.inode.file_len = 0;
			ret.modified = true;
		}
	}

	return ret;
}

template <class Config>
FileHandle<Config,SimpleFlashFsBase<Config>> SimpleFlashFsBase<Config>::allocate_free_inode_page_unmapped()
{
	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		typename Config::page_type page(header.page_size);

		ReadPageReturn ret = read_page( i, page, true );

		if( !ret && *ret.error != ReadError::ReadError ) {
			if( allocated_unwritten_pages.count(i) == 0 ) {
				file_handle_t fh_ret(this);
				fh_ret.page = i;
				allocated_unwritten_pages.insert(i);
				return fh_ret;
			}
		}
	}

	return {};
}

template <class Config>
FileHandle<Config,SimpleFlashFsBase<Config>> SimpleFlashFsBase<Config>::allocate_free_inode_page_mapped()
{
	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		ReadPageMappedReturn ret = read_page_mapped( i, header.page_size, true );

		if( !ret && *ret.error != ReadError::ReadError ) {
			if( allocated_unwritten_pages.count(i) == 0 ) {
				file_handle_t fh_ret(this);
				fh_ret.page = i;
				allocated_unwritten_pages.insert(i);
				return fh_ret;
			}
		}
	}

	return {};
}

template <class Config>
bool SimpleFlashFsBase<Config>::write_zero_pages( file_handle_t* file )
{
	for( auto & p : file->inode.data_pages ) {
		if( p.state == data_page_t::State::New ) {

			typename Config::page_type page(header.page_size);

			if( !write_page( file, page, p ) ) {
				// CPPDEBUG( Tools::static_format<100>( "cannot write zero page %d", p.page_id ));
				return false;
			}

			p.state = data_page_t::State::Stored;
		}
	}

	return true;
}

template <class Config>
bool SimpleFlashFsBase<Config>::flush( file_handle_t* file )
{
/*
	auto debug = []( file_handle_t* file ){
		std::vector<unsigned> v_valid;
		std::vector<unsigned> v_deleted;
		for( auto & i : file->inode.data_pages ) {
			if( i.state == data_page_t::State::Deleted ) {
				v_deleted.push_back(i.page_id);
			} else {
				v_valid.push_back(i.page_id);
			}
		}

		CPPDEBUG( Tools::static_format<100>( "writing inode %d,%d page %d (%s), data pages: %s, deleted pages: %s",
				file->inode.inode_number,
				file->inode.inode_version_number,
				file->page,
				file->inode.file_name,
				Tools::IterableToCommaSeparatedString(v_valid),
				Tools::IterableToCommaSeparatedString(v_deleted) ));
	};
*/
	if( !file->modified ) {
		return true;
	}

	if( file->inode.inode_number == 0) {
		file->inode.inode_number = max_inode_number + 1;
		max_inode_number = file->inode.inode_number;

		/*
		CPPDEBUG( Tools::static_format<100>( "writing new inode %d,%d at page %d",
				file->inode.inode_number,
				file->inode.inode_version_number,
				file->page));
		*/

		if( !write_zero_pages( file ) ) {
			CPPDEBUG( "failed flushing zero pages" );
		}

		auto page = inode2page(file->inode);
		add_page_checksum(page);

		// debug(file);

		if( !write_meta_page(file, page, file->page ) ) {
			/*
			CPPDEBUG( Tools::static_format<100>( "cannot write inode %d page %d",
					  file->inode.inode_number, file->page ) );
					  */
			return false;
		}
		file->modified = false;
		return true;
	}

	FileHandle old_file = file->get_disconnected_copy();

	file->page = allocate_free_inode_page_number();
	file->inode.inode_version_number++;

	auto page = inode2page(file->inode);
	add_page_checksum(page);

	if( !write_zero_pages( file ) ) {
		CPPDEBUG( "failed flushing zero pages" );
	}

	// debug(file);

	if( !write_meta_page(file, page, file->page ) ) {
		/*
		CPPDEBUG( Tools::static_format<100>( "cannot write inode %d,%d page %d",
				file->inode.inode_number, file->inode.inode_version_number, file->page ));
				*/
		return false;
	}

	file->modified = false;

	erase_inode_and_unused_pages(old_file, *file);

	return true;
}

template <class Config>
uint32_t SimpleFlashFsBase<Config>::allocate_free_inode_page_number_unmapped()
{
	typename Config::page_type page;
	page.reserve(header.page_size);

	for( unsigned i = 0; i < header.max_inodes; i++ ) {
		page.clear();
		page.resize(header.page_size);

		ReadPageReturn ret = read_page( i, page, true );

		if( !ret && *ret.error != ReadError::ReadError ) {
			if( allocated_unwritten_pages.count(i) == 0 ) {
				allocated_unwritten_pages.insert(i);
				return i;
			}
		}
	}

	return {};
}

template <class Config>
uint32_t SimpleFlashFsBase<Config>::allocate_free_inode_page_number_mapped()
{
	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		ReadPageMappedReturn ret = read_page_mapped( i, header.page_size, true );

		if( !ret && *ret.error != ReadError::ReadError ) {
			if( allocated_unwritten_pages.count(i) == 0 ) {
				allocated_unwritten_pages.insert(i);
				return i;
			}
		}
	}

	return {};
}

template <class Config>
uint32_t SimpleFlashFsBase<Config>::allocate_free_data_page()
{
	if( free_data_pages.empty() ) {
		CPPDEBUG( "no free data pages left" );
		return 0;
	}

	auto it = free_data_pages.begin();
	uint32_t ret = *it;
	free_data_pages.erase(it);

	return ret;
}

template <class Config>
uint32_t SimpleFlashFsBase<Config>::allocate_free_data_page( const file_handle_t *file )
{
	std::size_t max_pages = get_max_inode_data_pages( file );

	if( file->inode.data_pages.size() + 1 >= max_pages ) {
		// no space left
		return 0;
	}

	return allocate_free_data_page();
}

template <class Config>
std::size_t SimpleFlashFsBase<Config>::get_inode_data_space_size( const file_handle_t* file ) const
{
	return header.page_size - (sizeof(inode_t::inode_number)
			+ sizeof(inode_t::inode_version_number)
			+ sizeof(inode_t::file_name_len)
			+ file->inode.file_name_len
			+ sizeof(inode_t::attributes)
			+ sizeof(inode_t::file_len)
			+ sizeof(inode_t::pages));
}

template <class Config>
std::size_t SimpleFlashFsBase<Config>::get_max_inode_data_pages( const file_handle_t* file ) const
{
	std::size_t space = get_inode_data_space_size( file );
	return space / inode_t::data_pages_type_size;
}

template <class Config>
void SimpleFlashFsBase<Config>::erase_inode_and_unused_pages( file_handle_t & inode_to_erase,
		file_handle_t & next_inode_version )
{
/*
	CPPDEBUG( Tools::static_format<100>( "cleaning up inode %d,%d comparing with %d,%d",
			inode_to_erase.inode.inode_number,
			inode_to_erase.inode.inode_version_number,
			next_inode_version.inode.inode_number,
			next_inode_version.inode.inode_version_number));
*/

	// all pages, that are used by the next inode are still in use
	auto & dp = inode_to_erase.inode.data_pages;

	// assume to erase all old pages
	//typename Config::set_type<uint32_t> pages_to_erase( dp.begin(), dp.end() );
	PageSet<Config> pages_to_erase;
	for( auto & p : dp ) {
		pages_to_erase.unordered_insert( p.page_id );
	}

	// now remove all pages from the inode in the next version
	for( auto page : next_inode_version.inode.data_pages ) {
		pages_to_erase.erase(page.page_id);
	}

	// add the inode self too
	pages_to_erase.insert(inode_to_erase.page);

	for( auto page : pages_to_erase.get_data() ) {
		// CPPDEBUG( Tools::static_format<100>( "erasing page: %d", page ) );

		std::size_t address = header.page_size + page * header.page_size;
		mem->erase(address, header.page_size );

		if( page > header.max_inodes ) {
			free_data_pages.insert(page);
		}
	}

	inode_to_erase.modified = false;
}

template <class Config>
bool SimpleFlashFsBase<Config>::write_page( file_handle_t* file,
		const std::basic_string_view<std::byte> & page,
		typename Inode<Config>::data_page_t & page_meta )
{
	if( page_meta.state == data_page_t::State::New ) {
		std::size_t ret = mem->write( header.page_size + header.page_size * page_meta.page_id, page.data(), page.size() );

		if( ret == page.size() ) {
			page_meta.state = data_page_t::State::Stored;
			return true;
		} else {
			CPPDEBUG( "no space left on device" );
			//CPPDEBUG( Tools::static_format<100>("no space left on device ret: %d", ret ) );
			return false;
		}

	} else {
		uint32_t new_page_number = base::SimpleFlashFsBase<Config>::allocate_free_data_page();

		if( new_page_number == 0 ) {
			CPPDEBUG( "no space left on device" );
			return false;
		}

		uint32_t old_page_number = page_meta.page_id;

		std::size_t ret = mem->write( header.page_size + header.page_size * new_page_number, page.data(), page.size() );

		if( ret == page.size() ) {
			for( auto & p : file->inode.data_pages ) {
				if( p.page_id == old_page_number ) {
					p.page_id = new_page_number;
					p.state = data_page_t::State::Stored;
					break;
				}
			}
			return true;
		} else {
			CPPDEBUG( "no space left on device" );
			return false;
		}
	}
}

template <class Config>
bool SimpleFlashFsBase<Config>::allocate_new_data_pages( std::size_t page_idx, file_handle_t* file )
{
	// allocate new pages, at least the initial one
	while( page_idx >= file->inode.count_valid_data_pages() ) {
		uint32_t new_page_id = allocate_free_data_page(file);

		if( new_page_id == 0 ) {
			CPPDEBUG( "no space left on device" );
			return false;
		}

		// insert at the last valid (not deleted element)
		auto it = file->inode.data_pages.begin() + file->inode.count_valid_data_pages();

		file->inode.data_pages.insert( it, { new_page_id, data_page_t::State::New } );
	}

	return true;
}

template <class Config>
bool SimpleFlashFsBase<Config>::allocate_new_data_page_at( std::size_t page_idx, file_handle_t* file )
{
	uint32_t new_page_id = allocate_free_data_page(file);

	if( new_page_id == 0 ) {
		CPPDEBUG( "no space left on device" );
		return false;
	}

	/**
	 * Move the old data page to the end.
	 * We have to store any written page into an inode, so we know
	 * we are not allowed reusing it again.
	 */
	auto old_page_meta = file->inode.data_pages.at(page_idx);
	old_page_meta.state = data_page_t::State::Deleted;
	file->inode.data_pages.push_back( old_page_meta );

	file->inode.data_pages.at(page_idx) = { new_page_id, data_page_t::State::New };

	return true;
}

template <class Config>
std::size_t SimpleFlashFsBase<Config>::write( file_handle_t* file, const std::byte *data, std::size_t size )
{
	std::size_t page_idx = file->pos / header.page_size;
	std::size_t bytes_written = 0;

	/**
	 * store the data inside the inode itself, if there is enough space
	 */
	std::size_t space_inside_the_inode = get_inode_data_space_size(file);
	if( space_inside_the_inode > file->pos + size &&
		file->inode.file_len < space_inside_the_inode ) {

		if( file->inode.inode_data.size() < space_inside_the_inode ) {
			file->inode.inode_data.resize(space_inside_the_inode);
		}

		memcpy( file->inode.inode_data.data() + file->pos, data, size );
		file->pos += size;
		file->inode.file_len = std::max(static_cast<decltype(file->inode.file_len)>(file->pos), file->inode.file_len);
		file->modified = true;

		// the read write will occur during flush
		return size;

	}

	std::size_t additinal_bytes_written = 0;

	// copy data that was stored inside the inode itself to the first page
	if( file->inode.inode_data.size() ) {
		std::size_t origin_pos = file->pos;
		const std::size_t inode_data_size = origin_pos;
		file->pos = 0;
		file->modified = true;

		typename Config::page_type page(header.page_size);
		memcpy( page.data(), file->inode.inode_data.data(), inode_data_size );
		file->pos += inode_data_size;

		const std::size_t bytes_left = std::min( std::size_t(header.page_size - inode_data_size), size );
		memcpy( page.data() + file->pos, data, bytes_left );

		const std::size_t total_size = inode_data_size + bytes_left;

		file->pos = 0;
		// clear it to break recursion
		file->inode.inode_data.clear();
		const std::size_t subbytes_written = this->write( file, page.data(), total_size );

		if( subbytes_written != total_size ) {
			return 0;
		}

		if( total_size <= header.page_size ) {
			if( size == bytes_left ) {
				// all bytes written, nothing left to write
				return size;
			}
		}

		data                    += bytes_left;
		size                    -= bytes_left;
		additinal_bytes_written += bytes_left;
	}

	// unaligned data, map the buffer to a complete page
	const std::size_t data_start_at_page = file->pos % header.page_size;

	if( data_start_at_page != 0 ) {

		// allocated stall pages, if required
		if( page_idx > 0 ) {
			if( !allocate_new_data_pages( page_idx - 1, file ) ) {
				return 0;
			}
		}

		bool do_read_page = true;

		if( page_idx < file->inode.data_pages.size() &&
			file->inode.data_pages.at(page_idx).state == data_page_t::State::New ) {

			// do not read the page from disc.
			// do nothing on page structure, so it will automaticalle inited with zeros
			do_read_page = false;
		}

		if( do_read_page ) {
			// allocate a new page
			if( !allocate_new_data_pages( page_idx, file ) ) {
				return 0;
			}
		}

		typename Config::page_type page(header.page_size);
		const std::size_t page_number = file->inode.data_pages.at(page_idx).page_id;

		if( do_read_page ) {
			if( !read_page( page_number, page, false ) ) {
				//CPPDEBUG( Tools::static_format<100>( "reading from pos %d failed", page_number * header.page_size ) );
				CPPDEBUG( "reading page failed" );
				return 0;
			}
		}

		const std::size_t len = std::min( size, static_cast<size_t>(header.page_size - data_start_at_page) );
		memcpy( &page[data_start_at_page], data, len );

		if( file->inode.data_pages.at(page_idx).state == data_page_t::State::Stored ) {
			if( !allocate_new_data_page_at( page_idx, file ) ) {
				return 0;
			}
		}


		if( !write_page( file, page, file->inode.data_pages.at(page_idx) ) ) {
			CPPDEBUG( "no space left on device" );
			return 0;
		}


		bytes_written += len;
		file->pos += len;
	}

	while( bytes_written < size ) {
		page_idx = file->pos / header.page_size;

		if( !allocate_new_data_pages( page_idx, file ) ) {
			return 0;
		}

		// last partial page
		if( bytes_written + header.page_size > size ) {
			typename Config::page_type page(header.page_size);

			const auto & page_meta = file->inode.data_pages.at(page_idx);

			if( page_meta.state == data_page_t::State::Stored ) {
				if( !read_page( page_meta.page_id, page, false ) ) {
					// CPPDEBUG( Tools::static_format<100>( "reading from pos %d failed", page_meta.page_id * header.page_size ) );
					CPPDEBUG( "reading page failed" );
					return 0;
				}
			}

			const std::size_t len = std::min( static_cast<uint32_t>(size - bytes_written), header.page_size );
			memcpy( page.data(), data + bytes_written, len );

			if( page_meta.state == data_page_t::State::Stored ) {
				if( !allocate_new_data_page_at( page_idx, file ) ) {
					return 0;
				}
			}

			//CPPDEBUG( Tools::static_format<100>("writing at page: %d", file->inode.data_pages.at(page_idx).page_id ) );
			if( !write_page( file, page, file->inode.data_pages.at(page_idx) ) ) {
				CPPDEBUG( "no space left on device" );
				return 0;
			}

			bytes_written += len;
			file->pos += len;

		} else {

			std::basic_string_view<std::byte> page( data + bytes_written, header.page_size );

			if( !write_page( file, page, file->inode.data_pages.at(page_idx) ) ) {
				CPPDEBUG( "no space left on device" );
				return 0;
			}

			bytes_written += header.page_size;
			file->pos += header.page_size;

		} // else

	} // while

	file->modified = true;
	file->inode.file_len = std::max( static_cast<decltype(file->inode.file_len)>(file->pos), file->inode.file_len) ;

	return bytes_written + additinal_bytes_written;
}

template <class Config>
std::size_t SimpleFlashFsBase<Config>::read( file_handle_t* file, std::byte *data, std::size_t size )
{
	std::size_t page_idx = file->pos / header.page_size;
	std::size_t bytes_readen = 0;

	if( file->inode.file_len - file->pos < size ) {
		size = file->inode.file_len - file->pos;
	}

	if( size == 0 ) {
		return 0;
	}

	/**
	 * stored data is inside the inode itself, if there is enough space
	 */
	std::size_t space_inside_the_inode = get_inode_data_space_size(file);
	if( space_inside_the_inode > file->pos + size &&
		file->inode.file_len < space_inside_the_inode ) {

		if( file->inode.inode_data.empty() && !file->inode.data_pages.empty() ) {
			//CPPDEBUG( Tools::static_format<100>( "file: '%s' no data in inode, but would fit", file->inode.file_name ) );

		} else if( !file->inode.inode_data.empty() ) {
			std::size_t len = std::min( static_cast<decltype(size)>(file->inode.file_len - file->pos), size );
			memcpy( data, file->inode.inode_data.data() + file->pos, len );
			file->pos += len;
			return len;

		} else {
			//CPPDEBUG( Tools::static_format<100>( "invalid file '%s' no inode, no data pages", file->inode.file_name ) );
		}
	}

	// unaligned data, map the buffer to a complete page
	const std::size_t data_start_at_page = file->pos % header.page_size;

	if( data_start_at_page != 0 ) {

		// file corrupt
		if( page_idx >= file->inode.data_pages.size() ) {
			/*
			CPPDEBUG( Tools::static_format<100>( "less amount of data pages than expectd: data_pages: %d expected: %d",
					file->inode.data_pages.size(), page_idx + 1 ) );
					*/
			return bytes_readen;
		}

		auto & page_meta = file->inode.data_pages.at(page_idx);
		const std::size_t len = std::min( size, static_cast<size_t>(header.page_size - data_start_at_page) );

		if( page_meta.state == data_page_t::State::New ) {
			// this page contains only zeros
			memset( data + bytes_readen, 0, len );

		} else {
			typename Config::page_type page(header.page_size);
			if( !read_page( page_meta.page_id, page, false ) ) {
				//CPPDEBUG( Tools::static_format<100>( "reading from pos %d failed", page_idx * header.page_size ) );
				return bytes_readen;
			}

			memcpy( data + bytes_readen, &page[data_start_at_page], len );
		}

		bytes_readen += len;
		file->pos += len;

	}

	while( bytes_readen < size ) {
		page_idx = file->pos / header.page_size;

		// file corrupt
		if( page_idx >= file->inode.data_pages.size() ) {
			/*
			CPPDEBUG( Tools::static_format<100>( "less amount of data pages than expectd: data_pages: %d expected: %d",
					file->inode.data_pages.size(), page_idx + 1 ) );
					*/
			return bytes_readen;
		}

		auto & page_meta = file->inode.data_pages.at(page_idx);

		// last partial page
		if( bytes_readen + header.page_size > size ) {

			typename Config::page_type page(header.page_size);

			// if the page is unwritten, it contains only zeros
			if( page_meta.state == data_page_t::State::Stored ) {
				if( !read_page( page_meta.page_id, page, false ) ) {
					//CPPDEBUG( Tools::static_format<100>( "reading from pos %d failed", page_idx * header.page_size ) );
					return bytes_readen;
				}
			}

			const std::size_t len = std::min( static_cast<uint32_t>(size - bytes_readen), header.page_size );
			memcpy( data + bytes_readen, page.data(), len );

			bytes_readen += len;
			file->pos += len;

		} else {

			if( page_meta.state == data_page_t::State::New ) {
				// if the page is unwritten, it contains only zeros
				memset( data + bytes_readen, 0, header.page_size );
			}
			else if( !read_page( page_meta.page_id, data + bytes_readen, header.page_size ) ) {
				CPPDEBUG( "reading from device failed" );
				return bytes_readen;
			}

			bytes_readen += header.page_size;
			file->pos += header.page_size;

		} // else

	} // while

	return bytes_readen;
}

template <class Config>
bool SimpleFlashFsBase<Config>::delete_file( file_handle_t* file )
{
	// delete it by setting the filename to an empty string
	// next cleanup process will skip it
	file->inode.file_name.clear();
	file->inode.file_name_len = 0;

	file->inode.pages = 0;
	file->inode.data_pages.clear();
	file->inode.inode_data.clear();
	file->inode.file_len = 0;
	file->modified = true;

	if( !file->flush() ) {
		return false;
	}

	// invalidate the handle
	file->disconnect();

	return true;
}

template <class Config>
bool SimpleFlashFsBase<Config>::rename_file( file_handle_t* file, const std::string_view & new_file_name )
{
	auto other_file = find_file( new_file_name );

	if( other_file.valid() ) {
		other_file.delete_file();
	}

	file->inode.file_name = new_file_name;
	file->inode.file_name_len = file->inode.file_name.size();
	file->modified = true;
	file->flush();

	return true;
}

template <class Config>
bool SimpleFlashFsBase<Config>::enlarge_file( file_handle_t* file, std::size_t amount )
{
	const std::size_t target_size = file->inode.file_len + amount;

	if( amount >= get_number_of_free_data_pages() * header.page_size ) {
		CPPDEBUG( "cannot enlarge file, no free data pages left" );
		return false;
	}

	if( target_size >= get_max_file_size() ) {
		CPPDEBUG( "cannot enlarge file, target size > filesystem max size" );
		return false;
	}

	const std::size_t page_idx = target_size / header.page_size;

	const std::size_t space_inside_the_inode = get_inode_data_space_size(file);

	if( space_inside_the_inode >= target_size ) {
		if( file->inode.inode_data.size() < space_inside_the_inode ) {
			file->inode.inode_data.resize(space_inside_the_inode);
		}

	} else {
		if( !allocate_new_data_pages( page_idx, file ) ) {
			return false;
		}
	}

	file->modified = true;
	file->inode.file_len += amount;
	return true;
}

} // namespace base

} // namespace SimpleFlashFs


