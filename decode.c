#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"
#include "types.h"

char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    for (int i = 0; i < 8; i++)
    {
        data = (data << 1) | (image_buffer[i] & 1);
    }
    return data;
}

Status decode_data_from_image(char *data, int size, FILE *fptr_image)
{
    char image_buffer[8];
    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, sizeof(char), 8, fptr_image) != 8)
        {
            fprintf(stderr, "ERROR: Failed to read image data for decoding\n");
            return e_failure;
        }
        data[i] = decode_byte_from_lsb(image_buffer);
    }
    return e_success;
}

Status decode_magic_string(const char *magic_string, FILE *fptr_image)
{
    char decoded_magic[3]; 
    if (decode_data_from_image(decoded_magic, strlen(magic_string), fptr_image) == e_failure)
    {
        return e_failure;
    }

    decoded_magic[strlen(magic_string)] = '\0'; 

    printf("INFO: Decoded magic string: %s\n", decoded_magic); 

    if (strcmp(decoded_magic, magic_string) != 0)
    {
        fprintf(stderr, "ERROR: Magic string mismatch\n");
        return e_failure;
    }
    return e_success;
}

Status decode_secret_file_extn(char *file_extn, FILE *fptr_image)
{
    if (decode_data_from_image(file_extn, MAX_FILE_SUFFIX, fptr_image) == e_failure)
    {
        return e_failure;
    }
    file_extn[MAX_FILE_SUFFIX - 1] = '\0'; 
    printf("INFO: Decoded file extension: %s\n", file_extn); 
    return e_success;
}

Status decode_secret_file_size(long *file_size, FILE *fptr_image)
{
    char size_buffer[sizeof(long)];
    if (decode_data_from_image(size_buffer, sizeof(long), fptr_image) == e_failure)
    {
        return e_failure;
    }
    memcpy(file_size, size_buffer, sizeof(long)); 
    printf("INFO: Decoded file size: %ld\n", *file_size); 
    return e_success;
}

Status decode_secret_file_data(FILE *fptr_image, FILE *fptr_output, long file_size)
{
    char data;
    for (long i = 0; i < file_size; i++)
    {
        if (decode_data_from_image(&data, 1, fptr_image) == e_failure)
        {
            return e_failure;
        }
        if (fwrite(&data, sizeof(char), 1, fptr_output) != 1)
        {
            fprintf(stderr, "ERROR: Failed to write decoded data\n");
            return e_failure;
        }
    }
    printf("INFO: Secret data successfully decoded\n"); 
    return e_success;
}

Status do_decoding(const char *stego_image_fname, const char *output_fname)
{
    FILE *fptr_stego_image = fopen(stego_image_fname, "r");
    if (fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open stego image file %s\n", stego_image_fname);
        return e_failure;
    }

    FILE *fptr_output = fopen(output_fname, "w");
    if (fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open output file %s\n", output_fname);
        fclose(fptr_stego_image);
        return e_failure;
    }

    fseek(fptr_stego_image, 54, SEEK_SET);

    printf("INFO: File position after skipping BMP header: %ld\n", ftell(fptr_stego_image));

    if (decode_magic_string(MAGIC_STRING, fptr_stego_image) == e_failure)
    {
        fclose(fptr_stego_image);
        fclose(fptr_output);
        return e_failure;
    }

    printf("INFO: Magic string successfully decoded\n");

    char file_extn[MAX_FILE_SUFFIX];
    if (decode_secret_file_extn(file_extn, fptr_stego_image) == e_failure)
    {
        fclose(fptr_stego_image);
        fclose(fptr_output);
        return e_failure;
    }

    long file_size;
    if (decode_secret_file_size(&file_size, fptr_stego_image) == e_failure)
    {
        fclose(fptr_stego_image);
        fclose(fptr_output);
        return e_failure;
    }

    if (decode_secret_file_data(fptr_stego_image, fptr_output, file_size) == e_failure)
    {
        fclose(fptr_stego_image);
        fclose(fptr_output);
        return e_failure;
    }

    fclose(fptr_stego_image);
    fclose(fptr_output);
    return e_success;
}
