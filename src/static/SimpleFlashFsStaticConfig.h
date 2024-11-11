/**
* Internal data representation structs
* @author Copyright (c) 2023-2024 Martin Oberzalek
*/
#pragma once

#include "../SimpleFlashFsConstants.h"

#include <static_vector.h>
#include <static_string.h>
#include <static_list.h>

namespace SimpleFlashFs::static_memory {

    template <size_t SFF_FILE_NAME_MAX, size_t SFF_PAGE_SIZE, size_t SFF_MAX_PAGES, size_t SFF_MAX_SIZE>
    struct Config
    {
      using magic_string_type = Tools::static_string<MAGICK_STRING_LEN>;
      using string_type = Tools::static_string<SFF_FILE_NAME_MAX>;
      using string_view_type = std::string_view;
      using page_type = Tools::static_vector<std::byte,SFF_PAGE_SIZE>;
      constexpr static size_t PAGE_SIZE = SFF_PAGE_SIZE;
      constexpr static size_t MAX_PAGES = SFF_MAX_PAGES;
      constexpr static size_t FILE_NAME_MAX = SFF_FILE_NAME_MAX;
      constexpr static size_t MAX_SIZE = SFF_MAX_SIZE;

      template<class T> class vector_type : public Tools::static_vector<T,SFF_MAX_PAGES> {};

      static uint32_t crc32( const std::byte *bytes, size_t len );
    };

} // namespace SimpleFlashFs::static_memory