#include <stdlib.h>
#include <stdint.h>
#include <emmintrin.h>

void window_v1(uint8_t* img, size_t x, size_t y, size_t width, size_t height, uint8_t* result) {
    int32_t image_width = *(img + 18); // width of the pixel image
    image_width += *(img + 19) << 8;   // width is stored in reverse that's why we have to calculate the actual width
    image_width += *(img + 20) << 16;
    image_width += *(img + 21) << 24;

    int32_t image_height = *(img + 22); // height of the pixel image:
    image_height += *(img + 23) << 8;   // height is stored in reverse that's why we have to calculate the actual height
    image_height += *(img + 24) << 16;
    image_height += *(img + 25) << 24;

    // bytes per pixel = 3 Bytes
    int32_t imgStride = abs(image_width) * 3; // bytes per row in img
    int32_t alignedByte = (imgStride % 4) == 0 ? 0 : 4 - (imgStride % 4); // the bytes that have been added in the image for row alignment (in each row)
    imgStride += alignedByte; // now imgStride is the correct number of Bytes in each row including 4 Byte-alignment

    uint8_t* startAddress = img + *(img + 10); // start address of the array which contains the pixels

    size_t resultOffset = 0;

    if (image_height < 0) { // Top-down approach (the first pixel in the array is in the top left)
        for (size_t row = y; row < y + height; row++) {
            size_t imgRowOffset = row * imgStride; // starting Byte of each row

            size_t column = x;
            for (; column + 6 <= x + width; column += 6) {
                size_t imgColumnOffset = column * 3;
                size_t imgOffset = imgRowOffset + imgColumnOffset;

                __m128i data = _mm_loadu_si128((__m128i*)(startAddress + imgOffset)); // loading 120 Bits(15 Bytes) (5 Pixels) and 1/3 of a pixel --> 16 Bytes in total
                _mm_storeu_si128((__m128i*)(result + resultOffset), data); // storing 5 and 1/3 pixels using intrinsics

                resultOffset += 16;

                for (size_t k = 0; k < 2; k++) { // for the remaining two bytes of the 6. pixel
                    result[resultOffset] = startAddress[imgOffset + 16 + k];
                    resultOffset++;
                }
            }

            if (column < x + width) { // for the remaining Bytes of the pixel that couldn't be stored using intrinsics
                size_t remainingBytes = (x + width - column) * 3;
                size_t imgColumnOffset = column * 3;
                size_t imgOffset = imgRowOffset + imgColumnOffset;

                for (size_t l = 0; l < remainingBytes; l++) {
                    result[resultOffset] = startAddress[imgOffset + l];
                    resultOffset++;
                }
            }
        }
    } else { // (image_height > 0) Bottom-up approach (the first pixel in the array is in the bottom left)
        for (size_t row = y; row < y + height; row++) {
            size_t imgRowOffset = (abs(image_width) * abs(image_height) * 3 + alignedByte * abs(image_height)) - ((row + 1) * imgStride);

            size_t column = x;
            for (; column + 6 <= x + width; column += 6) {
                size_t imgColumnOffset = column * 3;
                size_t imgOffset = imgRowOffset + imgColumnOffset;

                __m128i data = _mm_loadu_si128((__m128i*)(startAddress + imgOffset)); // loading 120 Bits(15 Bytes) (5 Pixels) and 1/3 of a pixel --> 16 Bytes in total
                _mm_storeu_si128((__m128i*)(result + resultOffset), data); // storing 5 and 1/3 pixels using intrinsics

                resultOffset += 16;

                for (size_t k = 0; k < 2; k++) { // for the remaining two bytes of the 6. pixel
                    result[resultOffset] = startAddress[imgOffset + 16 + k];
                    resultOffset++;
                }
            }

            if (column < x + width) { // for the remaining Bytes of the pixel that couldn't be stored using intrinsics
                size_t remainingBytes = (x + width - column) * 3;
                size_t imgColumnOffset = column * 3;
                size_t imgOffset = imgRowOffset + imgColumnOffset;

                for (size_t l = 0; l < remainingBytes; l++) {
                    result[resultOffset] = startAddress[imgOffset + l];
                    resultOffset++;
                }
            }
        }
    }
}
