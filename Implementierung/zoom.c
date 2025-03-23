#include <stdint.h>
#include <stdlib.h>
#include <math.h>

size_t customRound(double value) {
    double integerPart;
    double decimal = modf(value, &integerPart);
    if (decimal <= 0.5) {
        return (size_t)integerPart;
    } else {
        return (size_t)integerPart + 1;
    }
}

size_t customFloor(double value, size_t scale_factor) {
    double integer;
    double decimal = modf(value, &integer);

    if (decimal < 0.95 - ( 1.0 / (double)scale_factor)) {
        return (size_t)integer;
    } else {
        return (size_t)integer + 1;
    }
}

void zoom(const uint8_t* img, size_t width, size_t height, size_t scale_factor, uint8_t* result){
    if (img == NULL) {  // no pixels
        return;
    }

    size_t scaled_height = height * scale_factor;
    size_t scaled_width = width * scale_factor;

    // place pixels to corresponding spots on the scaled matrix
    for (size_t row = 0; row < scaled_height; ++row) {
        for (size_t col = 0; col < scaled_width; ++col) {
            size_t original_row = row / scale_factor;
            size_t original_col = col / scale_factor;
            size_t original_index = original_row * width + original_col;
            size_t scaled_index = (row * scaled_width) + col;

            if (col % scale_factor == 0 && row % scale_factor == 0) {
                // Copy the original pixel value to the scaled matrix
                result[scaled_index * 3]= img[original_index * 3];
                result[(scaled_index * 3) + 1]= img[(original_index * 3) + 1];
                result[(scaled_index * 3) + 2]= img[(original_index * 3) + 2];
            } else {
                // Set the empty spots to 0
                result[scaled_index * 3] = 0;
                result[(scaled_index * 3) + 1] = 0;
                result[(scaled_index * 3) + 2] = 0;

            }
        }
    }

    for (size_t row = 0; row < scaled_height; row++) {
        for (size_t col = 0; col < scaled_width; col++) {
            size_t original_row = customRound((double)row / (double)scale_factor);
            size_t original_col = customRound((double)col / (double)scale_factor);
            size_t original_index = original_row * width + original_col;
            size_t scaled_index = row * scaled_width + col;

                // Copy the original pixel value to the scaled matrix
                if (scale_factor == 2) {
                    result[scaled_index * 3] = img[original_index * 3];
                    result[(scaled_index * 3) + 1] = img[(original_index * 3) + 1];
                    result[(scaled_index * 3) + 2] = img[(original_index * 3) + 2];
                } else // Check if the current position is a direct neighbor of the original pixel
                if ((abs((int)row - (int)(original_row * scale_factor)) <= 1 && abs((int)col - (int)(original_col * scale_factor)) <= 1) ) {
                    //if edge pixel
                    if (col == scaled_width - 1 || row == scaled_height - 1) {
                        result[scaled_index * 3] = 0;
                        result[(scaled_index * 3) + 1] = 0;
                        result[(scaled_index * 3) + 2] = 0;
                    } else {
                        result[scaled_index * 3] = img[original_index * 3];
                        result[(scaled_index * 3) + 1] = img[(original_index * 3) + 1];
                        result[(scaled_index * 3) + 2] = img[(original_index * 3) + 2];
                    }
                }

        }
    }



    for (size_t row = 0; row < scaled_height; row++) {
        for (size_t col = 0; col < scaled_width; col++) {
            size_t original_row = customFloor((double)row / (double)scale_factor, scale_factor);
            size_t original_col = customFloor((double)col / (double)scale_factor, scale_factor);
            size_t original_index = original_row * width + original_col;
            size_t scaled_index = row * scaled_width + col;

            if (result[scaled_index * 3] == 0 && result[(scaled_index * 3) + 1] == 0 && result[(scaled_index * 3) + 2] == 0) {
                // Check if the current spot is blank (RGB values are 0)

                if (row == scaled_height - 1) { // If down edge, take color above
                    result[scaled_index * 3] = result[((row - 1) * scaled_width + col ) * 3];
                    result[(scaled_index * 3) + 1] = result[(((row - 1) * scaled_width + col ) * 3) + 1];
                    result[(scaled_index * 3) + 2] = result[(((row - 1) * scaled_width + col ) * 3) + 2];
                } else if ( col == scaled_width - 1) { // if right edge, take color left
                    result[scaled_index * 3] = result[(row  * scaled_width + (col - 1) ) * 3];
                    result[(scaled_index * 3) + 1] = result[((row * scaled_width + (col - 1) ) * 3) + 1];
                    result[(scaled_index * 3) + 2] = result[((row * scaled_width + (col - 1) ) * 3) + 2];
                } else { // else take the color of the calculated original pixel
                    // Copy the original pixel value to the scaled matrix
                    result[scaled_index * 3] = img[original_index * 3];
                    result[(scaled_index * 3) + 1] = img[(original_index * 3) + 1];
                    result[(scaled_index * 3) + 2] = img[(original_index * 3) + 2];
                }

            }
        }
    }


}
