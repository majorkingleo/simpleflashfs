/**
 * SimpleFlashFs wrapper class for fopen, fwrite, fread, fclose, fflush
 * @author Copyright (c) 2024 Martin Oberzalek
 */

#pragma once
#ifndef SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICWRAPPER_H_
#define SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICWRAPPER_H_

#include <stdint.h>
#include <stddef.h>

#ifndef __cplusplus
extern "C" {
#endif

extern int SIMPLE_FLASH_FS_DYNAMIC_EOF;

struct SIMPLE_FLASH_FS_DYNAMIC_FILE;

/**
 * define SimpleFlashFs instance, that the wrapper is working on
 */
void SimpleFlashFs_dynamic_wrapper_register_default_instance_name( const char *name );
void SimpleFlashFs_dynamic_wrapper_unregister_default_instance_name( const char *name );

/**
 * fopen implemententation
 * path: filename and path
 * mode: rwa+
 */
SIMPLE_FLASH_FS_DYNAMIC_FILE* SimpleFlashFs_dynamic_fopen( const char *path, const char *mode);

/**
 * fclose implementation.
 * return values: 0 on success
 */
int SimpleFlashFs_dynamic_fclose( SIMPLE_FLASH_FS_DYNAMIC_FILE* stream );

/**
 * fflush implementation.
 * return values: 0 on success
 */
int SimpleFlashFs_dynamic_fflush( SIMPLE_FLASH_FS_DYNAMIC_FILE* stream );

/**
 * fwrite implementation.
 * return values: the number of bytes written to stream
 */
size_t SimpleFlashFs_dynamic_fwrite(const void *ptr,
             size_t size, size_t nmemb,
             SIMPLE_FLASH_FS_DYNAMIC_FILE * stream);

/**
 * fread implementation.
 * return values: the number of bytes read to stream
 */
size_t SimpleFlashFs_dynamic_fread(void *ptr,
             size_t size, size_t nmemb,
             SIMPLE_FLASH_FS_DYNAMIC_FILE * stream);

/**
 * feof implementation.
 * return values: nonzero if the end‐of‐file indicator is set for the stream
 */
int SimpleFlashFs_dynamic_feof( SIMPLE_FLASH_FS_DYNAMIC_FILE* stream );

/**
 * fgetc implementation.
 * return values: nonzero if the end‐of‐file indicator is set for the stream
 */
int SimpleFlashFs_dynamic_fgetc( SIMPLE_FLASH_FS_DYNAMIC_FILE *stream );

/**
 * fgets implementation.
 * return values: nonzero if the end‐of‐file indicator is set for the stream
 */
char *SimpleFlashFs_dynamic_fgets(char *s, int size, SIMPLE_FLASH_FS_DYNAMIC_FILE *stream);

/**
 * fputc implementation.
 * return values: return the character written as an unsigned char cast to an int or EOF on error
 */
int SimpleFlashFs_dynamic_fputc(int c, SIMPLE_FLASH_FS_DYNAMIC_FILE *stream);

/**
 * fputs implementation.
 * return values: return a nonnegative number on success, or EOF on error
 */
int SimpleFlashFs_dynamic_fputs(const char *s, SIMPLE_FLASH_FS_DYNAMIC_FILE *stream);

#ifndef __cplusplus
} // extern "C"
#endif


#endif /* SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICWRAPPER_H_ */
