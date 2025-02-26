#ifndef DECODE_H
#define DECODE_H

#include "types.h"
#include "common.h" 

Status decode_magic_string(const char *magic_string, FILE *fptr_image);
Status decode_secret_file_extn(char *file_extn, FILE *fptr_image);
Status decode_secret_file_size(long *file_size, FILE *fptr_image);
Status decode_secret_file_data(FILE *fptr_image, FILE *fptr_output, long file_size);
Status do_decoding(const char *stego_image_fname, const char *output_fname);

#endif
