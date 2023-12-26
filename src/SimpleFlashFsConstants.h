/*
 * SimpleFlashFsConstants.h
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */

#ifndef SRC_SIMPLEFLASHFSCONSTANTS_H_
#define SRC_SIMPLEFLASHFSCONSTANTS_H_

#include <cstdint>

namespace SimpleFlashFs {

extern const char * MAGICK_STRING;
extern const std::size_t MAGICK_STRING_LEN;

extern const char * ENDIANESS_LE;
extern const char * ENDIANESS_BE;

}

#endif /* SRC_SIMPLEFLASHFSCONSTANTS_H_ */
