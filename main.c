#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <encode|decode> [additional arguments]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "encode") == 0)
    {
        if (argc != 5)
        {
            fprintf(stderr, "Usage for encoding: %s encode <src_image> <secret_file> <stego_image>\n", argv[0]);
            return 1;
        }

        EncodeInfo encInfo;
        encInfo.src_image_fname = argv[2];
        encInfo.secret_fname = argv[3];
        encInfo.stego_image_fname = argv[4];

        if (open_files(&encInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: File opening failed\n");
            return 1;
        }

        encInfo.image_capacity = get_image_size_for_bmp(encInfo.fptr_src_image);
        encInfo.size_secret_file = get_file_size(encInfo.fptr_secret);

        if (check_capacity(&encInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: Insufficient capacity in source image\n");
            return 1;
        }

        if (do_encoding(&encInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: Encoding failed\n");
            return 1;
        }

        printf("INFO: Encoding successful, stego image created as %s\n", encInfo.stego_image_fname);
    }
    else if (strcmp(argv[1], "decode") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Usage for decoding: %s decode <stego_image> <output_file>\n", argv[0]);
            return 1;
        }

        if (do_decoding(argv[2], argv[3]) == e_failure)
        {
            fprintf(stderr, "ERROR: Decoding failed\n");
            return 1;
        }

        printf("INFO: Decoding successful, output written to %s\n", argv[3]);
    }
    else
    {
        fprintf(stderr, "Unsupported operation: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
