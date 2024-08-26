/**
 * SimpleFlashFs constants
 *
 * This file is part of the filesystem definition
 * and can be redistributed
 *
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */

#ifndef SRC_SIMPLEFLASHFSCONSTANTS_H_
#define SRC_SIMPLEFLASHFSCONSTANTS_H_

#include <cstdint>

namespace SimpleFlashFs {

extern const char * MAGICK_STRING;
constexpr std::size_t MAGICK_STRING_LEN = 13;

extern const char * ENDIANESS_LE;
extern const char * ENDIANESS_BE;
extern const std::size_t ENDIANESS_LEN;

constexpr std::size_t MIN_PAGE_SIZE = 37;

} // namespace SimpleFlashFs

#endif /* SRC_SIMPLEFLASHFSCONSTANTS_H_ */
