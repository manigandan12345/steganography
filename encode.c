#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "common.h"
#include "types.h"

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET); 
    fread(&width, sizeof(uint), 1, fptr_image);
    fread(&height, sizeof(uint), 1, fptr_image);
    printf("INFO: Image dimensions - Width: %u, Height: %u\n", width, height); 
    return width * height * 3; 
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END); 
    long size = ftell(fptr); 
    rewind(fptr);            
    printf("INFO: Secret file size: %ld bytes\n", size); 
    return size;
}

Status open_files(EncodeInfo *encInfo)
{
    printf("INFO: Opening source image file: %s\n", encInfo->src_image_fname);
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (!encInfo->fptr_src_image)
    {
        perror("fopen");
        return e_failure;
    }

    printf("INFO: Opening secret file: %s\n", encInfo->secret_fname);
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (!encInfo->fptr_secret)
    {
        perror("fopen");
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }

    printf("INFO: Opening stego image file: %s\n", encInfo->stego_image_fname);
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (!encInfo->fptr_stego_image)
    {
        perror("fopen");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        return e_failure;
    }

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    uint required_capacity = (54 + (strlen(MAGIC_STRING) + MAX_FILE_SUFFIX + sizeof(long) + encInfo->size_secret_file) * 8);
    printf("INFO: Image capacity: %u bytes, Required capacity: %u bytes\n", encInfo->image_capacity, required_capacity);
    if (encInfo->image_capacity < required_capacity)
    {
        fprintf(stderr, "ERROR: Insufficient capacity in source image\n");
        return e_failure;
    }
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    rewind(fptr_src_image);
    if (fread(header, sizeof(char), 54, fptr_src_image) != 54)
    {
        fprintf(stderr, "ERROR: Failed to read BMP header\n");
        return e_failure;
    }
    if (fwrite(header, sizeof(char), 54, fptr_dest_image) != 54)
    {
        fprintf(stderr, "ERROR: Failed to write BMP header\n");
        return e_failure;
    }
    printf("INFO: BMP header successfully copied\n"); 
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7 - i)) & 1);
    }
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char image_buffer[8];
    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, sizeof(char), 8, fptr_src_image) != 8)
        {
            fprintf(stderr, "ERROR: Failed to read image data for encoding\n");
            return e_failure;
        }
        encode_byte_to_lsb(data[i], image_buffer);
        if (fwrite(image_buffer, sizeof(char), 8, fptr_stego_image) != 8)
        {
            fprintf(stderr, "ERROR: Failed to write encoded data\n");
            return e_failure;
        }
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("INFO: Encoding magic string: %s\n", magic_string); 
    return encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image);
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[MAX_FILE_SUFFIX] = {0};
    strncpy(buffer, file_extn, MAX_FILE_SUFFIX - 1); 
    printf("INFO: Encoding file extension: %s\n", buffer); 
    return encode_data_to_image(buffer, MAX_FILE_SUFFIX, encInfo->fptr_src_image, encInfo->fptr_stego_image);
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char size_buffer[sizeof(long)];
    memcpy(size_buffer, &file_size, sizeof(long)); 
    printf("INFO: Encoding file size: %ld\n", file_size); 
    return encode_data_to_image(size_buffer, sizeof(long), encInfo->fptr_src_image, encInfo->fptr_stego_image);
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf("INFO: Encoding secret file data...\n"); 
    for (long i = 0; i < encInfo->size_secret_file; i++)
    {
        char data;
        if (fread(&data, sizeof(char), 1, encInfo->fptr_secret) != 1)
        {
            fprintf(stderr, "ERROR: Failed to read secret file data\n");
            return e_failure;
        }
        if (encode_data_to_image(&data, 1, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        {
            return e_failure;
        }
    }
    printf("INFO: Secret file data encoding completed\n"); 
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, sizeof(char), sizeof(buffer), fptr_src)) > 0)
    {
        if (fwrite(buffer, sizeof(char), bytes_read, fptr_dest) != bytes_read)
        {
            fprintf(stderr, "ERROR: Failed to copy remaining image data\n");
            return e_failure;
        }
    }
    printf("INFO: Remaining BMP data copied successfully\n"); 
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: Starting encoding process...\n");

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        return e_failure;
    }

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    if (encode_secret_file_data(encInfo) == e_failure)
    {
        return e_failure;
    }

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    printf("INFO: Encoding process completed successfully\n");
    return e_success;
}
