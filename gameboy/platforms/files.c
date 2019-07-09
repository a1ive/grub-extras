#include "../non_core/files.h"
#include "../non_core/logger.h"

#include "libs.h"

#include <stdio.h>
#include <stdint.h>

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/file.h>

/*  Given a file_path and buffer to store file data in, attempts to
 *  read the file into the buffer. Returns the size of the file if successful,
 *  returns 0 if unsuccessful. Buffer should be at minimum of size "MAX_FILE_SIZE"*/
unsigned long load_rom_from_file(const char *file_path, unsigned char *data) {

    // read byte by byte of ROM into memory
    grub_file_t file = 0;
	grub_size_t size = 0;
	file = grub_file_open (file_path, GRUB_FILE_TYPE_CAT);
	
	if (!file)
	{
		log_message(LOG_ERROR, "Error opening file %s\n", file_path);
		return 0;
	}
	size = grub_file_size (file);
	if (!size)
	{
		log_message(LOG_ERROR, "premature end of file %s\n", file_path);
		return 0;
	}

    uint32_t count = 0; 
    size_t read = 0; 
    //Read file contents into buffer
    while(count < MAX_FILE_SIZE && (read = grub_file_read(file, data, 4096)) > 0) {
        count += read;
        data += read;
    }

    if (count == 0) {
       log_message(LOG_WARN, "Empty file %s\n", file_path);
    }

    grub_file_close (file);
    
    return count; 
}


/* Given a file_path and buffer, attempts to load save data into the buffer
 * up to the suppled size in bytes. Returns the size of the file if successful, 
 * returns 0 if unsuccessful. Buffer should at least be of length size*/
unsigned long load_SRAM(const char *file_path __attribute__ ((unused)), unsigned char *data __attribute__ ((unused)), unsigned long size __attribute__ ((unused))) {
    return 0;
}
 

/* Given a file_path, save data and the size of save data, attempts to
 * save the data to the given file. Returns 1 if successful, 0 otherwise */
int save_SRAM(const char *file_path __attribute__ ((unused)), const unsigned char *data __attribute__ ((unused)), unsigned long size __attribute__ ((unused))) {
    return 0;
}            
