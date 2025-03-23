#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include "window.h"
#include "window_v1.h"
#include "zoom.h"


int main(int argc, char **argv) {

    const char* help_message = //help message that describes all options
            "Positional Arguments:\n"
            "The input BMP-File\n"
            "\n"
            "Optional Arguments:\n"
            "-V X   Decides the implementation to be used. If this option is not set, it will be considered as V 0\n"
            "       (For X = 0 the naive implementation and for X = 1 the SIMD optimized implementation will be used)\n"
            "-B X   Calculates the runtime of the program for the given inputs.\n "
            "       The argument X determines the number of repetitions for the runtime calculation\n"
            "       (If no argument X is given, X is considered to be 1)\n"
            "-s X   The coordinates of the starting point of the algorithm in format -s <x>,<y>\n "
            "       (If this option is not set: x = 0, y = 0)\n"
            "-w X   The width of the window (If this option is not set: width = the width of the BMP-File)\n"
            "-h X   The height of the window (If this option is not set: height = the height of the BMP-File)\n"
            "-f X   The scale-factor used to zoom the image (If this option is not set: scale factor = 3) \n"
            "-o D   The name of the output file in format -o <filename>. The optional argument D must be a .bmp file\n"
            "       (If this option is not set: output file = output.bmp)\n"
            "-h     Prints out the description of all the available command line arguments\n"
            "--help Prints out the description of all the available command line arguments\n";


    static struct option long_options[] = { //in order to deal with long option --help
            {"help", no_argument, 0, 'h'}, //treating --help same as -h
            {NULL, 0, NULL, 0}};


    int option;

    int implementation_version = 0; // Selects one of two implementations. 0 = Main implementation with SIMD optimizations, 1 = Naive and unoptimized implementation (Default value = 0)
    char* imp_ver_endptr; //Used for error checking in strtol() statement

    _Bool test_mode = false; // If Option B is set than it calculates the runtime of the program for the given inputs
    int number_of_repetitions = 1; //How many times the program should be run during testing (Default value = 1)

    size_t x = 0; // Default value = 0
    size_t y = 0; // Default value = 0

    size_t width = 1; //filler value
    size_t height = 1; //filler value
    bool width_set = false;
    bool height_set = false;

    size_t scale_factor = 3; // Default value = 3

    char* output_file = "output.bmp"; // Holds the file name of the output file
    char* input_file; // Holds the file name of the given BMP-File

    while((option = getopt_long(argc, argv, ":V:B:s:w:h:f:o:h", long_options,NULL )) != -1) {
        switch (option) {
            case 'V':
                implementation_version = strtol(optarg, &imp_ver_endptr, 10); //Set the implementation version as the given argument

                if(imp_ver_endptr == optarg || *imp_ver_endptr != '\0' || implementation_version < 0 || implementation_version > 1) { //Invalid input if the given argument isn't 0 or 1
                    fprintf(stderr , "Invalid input for implementation version: %s\n ", optarg);
                    fprintf(stderr, "Please select 0 for the naive implementation or 1 for the with SIMD optimized implementation\n");
                    return 1;
                }
                break;

            case 'B' :
                errno = 0; //for overflow checking
                test_mode = true;
                char* number_of_repetitions_endptr; //used for error checking in strtol() statement

                if(optarg[0] != '-') {
                    number_of_repetitions = strtol(optarg, &number_of_repetitions_endptr, 10); //set number of runs as the given option argument
                    if(number_of_repetitions_endptr == optarg || *number_of_repetitions_endptr != '\0' || number_of_repetitions < 1 || errno == ERANGE) { //Invalid input if the given argument isn't a number, less than 1 or cannot fit in an integer
                        fprintf(stderr,"Invalid input for the number of program repetitions: %s\n ", optarg);
                        fprintf(stderr, "Number of program repetitions must be a number greater than 0\n");
                        return 1;
                    }
                } else {
                    if(strlen(optarg) > 1 && isdigit(optarg[1])) { //if a negative number is given for the program repetitions
                        fprintf(stderr,"Invalid input for the number of program repetitions: %s\n ", optarg);
                        fprintf(stderr, "Number of program repetitions must be a number greater than 0\n");
                        return 1;
                    } else {
                        optind -= 1;
                    }
                }
                break;

            case 's':
                errno = 0; //for overflow checking
                char* x_endptr; //for error-checking in strtol() statements
                char* y_endptr; //for error-checking in strtol() statements

                if(strlen(optarg) > 1 && optarg[0] == '-' && isdigit(optarg[1]) == false) { //in order to check if no argument for -s is given
                    fprintf(stderr, "Option -s must take an argument that is two numbers greater equal 0 split by a comma\n");
                    fprintf(stderr, "%s", help_message);
                    return 1;
                }

                char* input_string = strdup(optarg); //copying the optarg

                char* x_string = strtok(input_string, ","); //separating the string on ','
                if(x_string == NULL) {
                    fprintf(stderr, "Invalid input for the starting coordinates: %s \n", optarg);
                    fprintf(stderr, "The starting coordinates should consist of two numbers greater equal 0 with a comma in between");
                    return 1;
                }

                char* y_string = strtok(NULL, "\0"); //the string after the ','
                if(y_string == NULL) {
                    fprintf(stderr, "Invalid input for the starting coordinates: %s \n", optarg);
                    fprintf(stderr, "The starting coordinates should consist of two numbers greater equal 0 with a comma in between");
                    return 1;
                }


                if(x_string[0] == '-'){
                    if(strlen(x_string) > 1 && isdigit(x_string[1])) { //in order to check if a negative number is given for x
                        fprintf(stderr, "Invalid input for x-coordinate: %s\n ", x_string);
                        fprintf(stderr, "x must be a number that is greater equal 0 \n");
                    } else {
                        fprintf(stderr, "Option -s must take an argument that is two numbers greater equal 0 split by a comma\n");
                        fprintf(stderr, "%s", help_message);
                    }
                    return 1;
                }

                x = strtol(x_string, &x_endptr, 10); //set x as the given option argument
                if(x_endptr == optarg || *x_endptr != '\0' || errno == ERANGE) { //Invalid input if the given argument isn't a number or cannot fit in a size_t
                    fprintf(stderr, "Invalid input for x-coordinate: %s\n ", optarg);
                    fprintf(stderr, "x must be a number that is greater equal 0\n");
                    return 1;
                }

                if(y_string[0] == '-'){
                    if(strlen(y_string) > 1 && isdigit(y_string[1])) { //in order to check if a negative number is given for y
                        fprintf(stderr, "Invalid input for y-coordinate: %s\n ", y_string);
                        fprintf(stderr, "y must be a number that is greater equal 0 \n");
                    } else {
                        fprintf(stderr, "Option -s must take an argument that is two numbers greater equal 0 split by a comma\n");
                        fprintf(stderr, "%s", help_message);
                    }
                    return 1;
                }

                y = strtol(y_string, &y_endptr, 10); //set y as the given option argument
                if(y_endptr == optarg || *y_endptr != '\0' || errno == ERANGE) { //Invalid input if the given argument isn't a number or cannot fit in an integer
                    fprintf(stderr, "Invalid input for y-coordinate: %s\n ", optarg);
                    fprintf(stderr, "y must be a number that is greater equal 0 \n");
                    return 1;
                }

                break;

            case 'w':
                errno = 0; //for Overflow check
                char* width_endptr; //for error-checking in strtol() statements

                if(optarg[0] == '-'){
                    if(strlen(optarg) > 1 && isdigit(optarg[1])) { //if a negative number is given for width
                        fprintf(stderr, "Invalid input for width: %s\n ", optarg);
                        fprintf(stderr, "Width must be number that is greater than 0 \n");
                    } else {
                        fprintf(stderr, "Option -w must take an argument that is a number greater than 0\n");
                        fprintf(stderr, "%s", help_message);
                    }
                    return 1;
                }

                width = strtol(optarg, &width_endptr, 10); //set width as the given option argument
                if(width_endptr == optarg || *width_endptr != '\0' || width == 0 || errno == ERANGE) { //Invalid input if the given argument isn't a number, less than 1 or cannot fit in an integer
                    fprintf(stderr , "Invalid input for width: %s\n ", optarg);
                    fprintf(stderr, "Width must be number that is greater than 0 \n");
                    return 1;
                }

                width_set = true;
                break;

            case 'h':
                if(optarg != NULL) { //case for -h X
                    errno = 0; //for Overflow check
                    char *height_endptr; //for error-checking in strtol() statements

                    if(optarg[0] == '-'){
                        if(strlen(optarg) > 1 && isdigit(optarg[1])) { //if a negative number is given for height
                            fprintf(stderr, "Invalid input for height: %s\n", optarg);
                            fprintf(stderr, "Height must be a number that is greater than 0 \n");
                        } else {
                            fprintf(stderr, "Option -h must take an argument that is a number greater than 0\n");
                            fprintf(stderr, "%s", help_message);
                        }
                        return 1;

                    }

                    height = strtol(optarg, &height_endptr, 10); //set height as the given option argument
                    if (height_endptr == optarg || *height_endptr != '\0' || height == 0 || errno == ERANGE) { //Invalid input if the given argument isn't a number, less than 1 or cannot fit in an integer
                        fprintf(stderr, "Invalid input for height: %s\n", optarg);
                        fprintf(stderr, "Height must be a number that is greater than 0 \n");
                        return 1;
                    }

                    height_set = true;
                } else { //case for --help
                    fprintf(stderr, "%s", help_message);
                    return 0;
                }
                break;

            case 'f':
                errno = 0; //for Overflow check
                char* scale_factor_endptr; //for error-checking in strtol() statements

                if(optarg[0] == '-'){
                    if(strlen(optarg) > 1 && isdigit(optarg[1])) { //if a negative number is given for the scale factor
                        fprintf(stderr, "Invalid input for scale-factor: %s\n", optarg);
                        fprintf(stderr, "Scale factor must be a number that is greater than 0 \n");
                    } else {
                        fprintf(stderr, "Option -f must take an argument that is a number greater than 0 \n");
                        fprintf(stderr, "%s", help_message);
                    }
                    return 1;
                }

                scale_factor = strtol(optarg, &scale_factor_endptr, 10); //set scale_factor as the given option argument
                if(scale_factor_endptr == optarg || *scale_factor_endptr != '\0' || scale_factor == 0 || errno == ERANGE) { //Invalid input if the given argument isn't a number, less than 1 or cannot fit in an integer
                    fprintf(stderr, "Invalid input for scale-factor: %s\n", optarg);
                    fprintf(stderr, "Scale factor must be a number that is greater than 0 \n");
                    return 1;
                }
                break;

            case 'o':
                output_file = optarg; //assign the given file to output file
                if(!(strlen(output_file) > 3 && !strcmp(output_file + strlen(output_file) - 4, ".bmp"))) {
                    fprintf(stderr, "Invalid input for the output file: %s \n", optarg);
                    fprintf(stderr, "The output file must be a .bmp file");
                    return 1;
                }
                break;


            case ':':
                switch (optopt) {
                    case 'B': //case with -B without arguments
                        test_mode = true;
                        break;

                    case 'h': //case for -h
                        fprintf(stderr, "%s", help_message);
                        return 0;

                    default:
                        fprintf(stderr, "option -%c is missing a required argument\n", optopt);
                        return 1;
                }

            default:
                break;

        }
    }

    if(optind >= argc) { //Case where no positional argument is given
        fprintf(stderr, "Missing positional argument: Please enter a file or directory \n");
        return 1;
    } else {
        input_file = argv[optind];
        if(!(strlen(input_file) > 3 && !strcmp(input_file + strlen(input_file) - 4, ".bmp"))) {
            fprintf(stderr, "Invalid input for the input file: %s \n", input_file);
            fprintf(stderr, "The input file must be a .bmp file");
            return 1;
        }
    }

    struct timespec start; //for benchmarking
    if (test_mode == true) { //if Option -B is set
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    for (int i = 0; i < number_of_repetitions; i++) {

        FILE *input = fopen(input_file, "rb"); //Opening the file in read binary mode to read bytes from the file
        if (input == NULL) { //checking the correctness of the file
            fprintf(stderr, "Failed to open the input file %s: No such file or directory", input_file);
            return 1;
        }

        fseek(input, 0, SEEK_END); //moving the pointer to the end of the file
        long size = ftell(input); // calculating the size of the file
        rewind(input); //moving the file pointer to the beginning of the file

        uint8_t *image = malloc(size); //Allocating memory the image taken from the BMP-File
        if (image == NULL) {
            fprintf(stderr, "An error has occurred while allocating memory");
            fclose(input);
            return 1;
        }

        size_t read_bytes = fread(image, 1, size, input); //Copying bytes from the file to the allocated memory
        if (read_bytes != (size_t) size) {
            fprintf(stderr, "An error has occurred while reading from the file");
            free(image);
            fclose(input);
            return 1;
        }

        int32_t image_width = *(image + 18) +  (*(image + 19) << 8) + (*(image + 20) << 16) + (*(image + 21) << 24); // the bitmap width in pixels
        int32_t image_height = *(image + 22) +  (*(image + 23) << 8) + (*(image + 24) << 16) + (*(image + 25) << 24); //the bitmap height in pixels
        if (image_height < 0 ) {
            image_height = -image_height; //If the image height is given as negative
        }

        if (!width_set) {
            width = image_width; //Default value for the width of the window is the width of the image
        }

        if (!height_set) {
            height = image_height; //Default value for the height of the window is the height of the image
        }

        //Checking if x, y, width and height are appropriate for the image width and image height
        if ((int32_t) (x + width) > image_width) {
            if((int32_t) width > image_width) {
                fprintf(stderr, "The width of the window can not be bigger than the width of the image \n");
            } else {
                fprintf(stderr, "The window exceeded the boundaries of the image \n");
                fprintf(stderr, "The starting x-coordinate plus width must be less than or equal to the width of the image\n");
            }
            free(image);
            fclose(input);
            return 1;
        } else if ((int32_t) (y + height) > image_height) {
            if((int32_t) height > image_height) {
                fprintf(stderr, "The height of the window can not be bigger than the height of the image \n");
            } else {
                fprintf(stderr, "The window exceeded the boundaries of the image \n");
                fprintf(stderr, "The starting y-coordinate plus height must be less than or equal to the height of the image\n");
            }
            free(image);
            fclose(input);
            return 1;
        }

        uint8_t *window_result = malloc(width * height * 3); // Allocating memory for the window() function
        if (window_result == NULL) {
            fprintf(stderr, "An error has occurred while allocating memory");
            free(image);
            fclose(input);
            return 1;
        }

        size_t pixels_length = width * height * scale_factor * scale_factor * 3; // Size of pixels in bytes after the zoom()-function

        uint8_t *zoom_result = malloc(pixels_length); // Allocating memory for the zoom()-function
        if (zoom_result == NULL) {
            fprintf(stderr, "An error has occurred while allocating memory");
            free(image);
            fclose(input);
            free(window_result);
            return 1;
        }

        FILE *output = fopen(output_file, "wb");// Opening the file in binary mode for writing
        if (output == NULL) {
            fprintf(stderr, "Failed to open the output file %s: No such file or directory", output_file);
            free(window_result);
            free(zoom_result);
            free(image);
            fclose(input);
            return 1;
        }

        if (implementation_version == 0) { //The main implementation without SIMD optimizations
            window(image, x, y, width, height, window_result);
        } else { //The implementation with SIMD optimizations
            window_v1(image, x, y, width, height, window_result);
        }

        if (scale_factor > 1) {
            zoom(window_result, width, height, scale_factor, zoom_result);
        } else {
            zoom_result = window_result;
        }

        size_t new_width = width * scale_factor; //width of the image after the zoom()-function
        size_t new_height = height * scale_factor; //height of the image after the zoom()-function

        size_t padding = (new_width * 3) % 4 == 0 ? 0 : 4 - ((new_width * 3) % 4); //number of padding bytes at each row
        size_t file_size = 54 + pixels_length + (padding * new_width); // size of the file in bytes

        uint8_t file_header[14];
        uint8_t information_header[40];

        //Bytes 0-1: signature used for identifying BMP files
        file_header[0] = 'B';
        file_header[1] = 'M';

        //Bytes 2-5: size of the bmp file in bytes
        file_header[2] = file_size;
        file_header[3] = file_size >> 8;
        file_header[4] = file_size >> 16;
        file_header[5] = file_size >> 24;

        file_header[6] = 0;
        file_header[7] = 0;
        file_header[8] = 0;
        file_header[9] = 0;

        //Bytes 10-13: The byte where the pixels start
        file_header[10] = 54;
        file_header[11] = 0;
        file_header[12] = 0;
        file_header[13] = 0;

        //Bytes 14-17: Size of the information header
        information_header[0] = 40;
        information_header[1] = 0;
        information_header[2] = 0;
        information_header[3] = 0;

        //Bytes 18-21: The width of the bitmap in pixels
        information_header[4] = (width * scale_factor);
        information_header[5] = (width * scale_factor) >> 8;
        information_header[6] = (width * scale_factor) >> 16;
        information_header[7] = (width * scale_factor) >> 24;

        //Bytes 22-25: The height of the bitmap in pixels. Here given as negative in order to load the pixels top-down not bottom-up
        information_header[8] = -(height * scale_factor);
        information_header[9] = (-(height * scale_factor)) >> 8;
        information_header[10] = (-(height * scale_factor)) >> 16;
        information_header[11] = (-(height * scale_factor)) >> 24;

        information_header[12] = 1;
        information_header[13] = 0;

        //Bytes 28-29: Number of bits per pixel
        information_header[14] = 24;

        for (int k = 15; k < 23; k++) {
            information_header[k] = 0;
        }

        //Bytes 38-41: Vertical Resolution
        information_header[23] = 0xA4;
        information_header[24] = 0x0E;
        information_header[25] = 0;
        information_header[26] = 0;

        //Bytes 42-45: Horizontal Resolution
        information_header[27] = 0xA4;
        information_header[28] = 0x0E;
        information_header[29] = 0;
        information_header[30] = 0;

        for (int k = 31; k < 40; k++) {
            information_header[k] = 0;
        }

        fwrite(file_header, 1, 14, output);
        fwrite(information_header, 1, 40, output);

        if (padding == 0) {
            fwrite(zoom_result, 1, pixels_length, output);
        } else { //if padding is required
            uint8_t padding_bytes[] = {0, 0, 0 };
            size_t offset = 0;
            for (size_t l = 0; l < new_height; l++) {
                fwrite(zoom_result + offset, 1, new_width * 3, output);
                fwrite(padding_bytes, 1, padding, output);
                offset += (new_width * 3);
            }
        }

        free(window_result);
        if (scale_factor > 1) {
            free(zoom_result);
        }
        free(image);
        fclose(input);
        fclose(output);

    }

    if (test_mode == true) { //if Option -B is set
        struct timespec end; // for benchmarking
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time = end.tv_sec - start.tv_sec + 1e-9 *(end.tv_nsec - start.tv_nsec); //calculating the runtime in seconds
        printf("The program took %f seconds to complete \n", time / number_of_repetitions);
    }
    return 0;
}
