#ifndef ENCODE_H
#define ENCODE_H

#include "types.h"
#include "common.h" 

typedef struct _EncodeInfo
{
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;
    char image_data[8];

    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX]; 
    long size_secret_file;

    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;

Status open_files(EncodeInfo *encInfo);
uint get_image_size_for_bmp(FILE *fptr_image);
uint get_file_size(FILE *fptr);
Status check_capacity(EncodeInfo *encInfo);
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);
Status encode_secret_file_data(EncodeInfo *encInfo);
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);
Status encode_byte_to_lsb(char data, char *image_buffer);
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);
Status do_encoding(EncodeInfo *encInfo);

#endif
