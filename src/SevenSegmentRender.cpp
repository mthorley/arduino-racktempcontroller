#include "SevenSegmentRender.h"

void SevenSegmentRender::drawNumeric(const int x, const int y, const uint8_t n, 
                                     const FontSize_t size, const Rotation_t rotation) {
    switch(size) {
        case LARGE:
            switch(rotation) {
                case Rotation_t::ROT_0:
                    drawLargeNumeric(x, y, n);
                    break;
                
                default:
                    Log.error(F("Rotation not implemented"));
                    break;
            }

        case SMALL:
            switch(rotation) {
                case Rotation_t::ROT_0:
                    drawSmallNumeric(x, y, n);
                    break;

                case Rotation_t::ROT_90:
                    drawSmallNumericRotate90(y, x, n);
                    break;

                default:
                    Log.error(F("Rotation not implemented"));
                    break;
            }
            break;
    }
}

void SevenSegmentRender::drawSmallNumeric(const int x, const int y, const uint8_t n) {
    switch(n) {
        case 0:
            _oled.displayBMP(oled7seg_0_24bit_19x25, x, y);
            break;
        case 1:
            _oled.displayBMP(oled7seg_1_24bit_19x25, x, y);
            break;
        case 2:
            _oled.displayBMP(oled7seg_2_24bit_19x25, x, y);
            break;
        case 3:
            _oled.displayBMP(oled7seg_3_24bit_19x25, x, y);
            break;
        case 4:
            _oled.displayBMP(oled7seg_4_24bit_19x25, x, y);
            break;
        case 5:
            _oled.displayBMP(oled7seg_5_24bit_19x25, x, y);
            break;
        case 6:
            _oled.displayBMP(oled7seg_6_24bit_19x25, x, y);
            break;
        case 7:
            _oled.displayBMP(oled7seg_7_24bit_19x25, x, y);
            break;
        case 8:
            _oled.displayBMP(oled7seg_8_24bit_19x25, x, y);
            break;
        case 9:
            _oled.displayBMP(oled7seg_9_24bit_19x25, x, y);
            break;
        case BLANK:
            // display background '8' image
            _oled.displayBMP(oled7seg_base_24bit_19x25, x, y);
            break;

        default:
            break;
    }
}

void SevenSegmentRender::drawSmallNumericRotate90(const int x, const int y, const uint8_t n) {
    switch(n) {
        case 0:
            _oled.displayBMP(oled7seg_0_24bit_19x25_ROT90, x, y);
            break;
        case 1:
            _oled.displayBMP(oled7seg_1_24bit_19x25_ROT90, x, y);
            break;
        case 2:
            _oled.displayBMP(oled7seg_2_24bit_19x25_ROT90, x, y);
            break;
        case 3:
            _oled.displayBMP(oled7seg_3_24bit_19x25_ROT90, x, y);
            break;
        case 4:
            _oled.displayBMP(oled7seg_4_24bit_19x25_ROT90, x, y);
            break;
        case 5:
            _oled.displayBMP(oled7seg_5_24bit_19x25_ROT90, x, y);
            break;
        case 6:
            _oled.displayBMP(oled7seg_6_24bit_19x25_ROT90, x, y);
            break;
        case 7:
            _oled.displayBMP(oled7seg_7_24bit_19x25_ROT90, x, y);
            break;
        case 8:
            _oled.displayBMP(oled7seg_8_24bit_19x25_ROT90, x, y);
            break;
        case 9:
            _oled.displayBMP(oled7seg_9_24bit_19x25_ROT90, x, y);
            break;
        case BLANK:
            // display background '8' image
            _oled.displayBMP(oled7seg_base_24bit_19x25_ROT90, x, y);
            break;

        default:
            break;
    }
}

void SevenSegmentRender::drawLargeNumeric(const int x, const int y, const uint8_t n) {
    switch(n) {
        case 0:
            _oled.displayBMP(oled7seg_0_24bit_38x50, x, y);
            break;
        case 1:
            _oled.displayBMP(oled7seg_1_24bit_38x50, x, y);
            break;
        case 2:
            _oled.displayBMP(oled7seg_2_24bit_38x50, x, y);
            break;
        case 3:
            _oled.displayBMP(oled7seg_3_24bit_38x50, x, y);
            break;
        case 4:
            _oled.displayBMP(oled7seg_4_24bit_38x50, x, y);
            break;
        case 5:
            _oled.displayBMP(oled7seg_5_24bit_38x50, x, y);
            break;
        case 6:
            _oled.displayBMP(oled7seg_6_24bit_38x50, x, y);
            break;
        case 7:
            _oled.displayBMP(oled7seg_7_24bit_38x50, x, y);
            break;
        case 8:
            _oled.displayBMP(oled7seg_8_24bit_38x50, x, y);
            break;
        case 9:
            _oled.displayBMP(oled7seg_9_24bit_38x50, x, y);
            break;
        case BLANK:
            // display background '8' image
            _oled.displayBMP(oled7seg_base_24bit_38x50, x, y);
            break;

        default:
            break;
    }
}
