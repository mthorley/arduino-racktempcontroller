#ifndef __7SEGMENTRENDER_H
#define __7SEGMENTRENDER_H

#include <ArduinoLog.h>

#include <FTOLED.h>
#include "images/oled7seg-base-24bit-38x50.h"
#include "images/oled7seg-0-24bit-38x50.h"
#include "images/oled7seg-1-24bit-38x50.h"
#include "images/oled7seg-2-24bit-38x50.h"
#include "images/oled7seg-3-24bit-38x50.h"
#include "images/oled7seg-4-24bit-38x50.h"
#include "images/oled7seg-5-24bit-38x50.h"
#include "images/oled7seg-6-24bit-38x50.h"
#include "images/oled7seg-7-24bit-38x50.h"
#include "images/oled7seg-8-24bit-38x50.h"
#include "images/oled7seg-9-24bit-38x50.h"

#include "images/oled7seg-base-24bit-19x25.h"
#include "images/oled7seg-0-24bit-19x25.h"
#include "images/oled7seg-1-24bit-19x25.h"
#include "images/oled7seg-2-24bit-19x25.h"
#include "images/oled7seg-3-24bit-19x25.h"
#include "images/oled7seg-4-24bit-19x25.h"
#include "images/oled7seg-5-24bit-19x25.h"
#include "images/oled7seg-6-24bit-19x25.h"
#include "images/oled7seg-7-24bit-19x25.h"
#include "images/oled7seg-8-24bit-19x25.h"
#include "images/oled7seg-9-24bit-19x25.h"

#include "images/oled7seg-base-24bit-19x25-ROT90.h"
#include "images/oled7seg-0-24bit-19x25-ROT90.h"
#include "images/oled7seg-1-24bit-19x25-ROT90.h"
#include "images/oled7seg-2-24bit-19x25-ROT90.h"
#include "images/oled7seg-3-24bit-19x25-ROT90.h"
#include "images/oled7seg-4-24bit-19x25-ROT90.h"
#include "images/oled7seg-5-24bit-19x25-ROT90.h"
#include "images/oled7seg-6-24bit-19x25-ROT90.h"
#include "images/oled7seg-7-24bit-19x25-ROT90.h"
#include "images/oled7seg-8-24bit-19x25-ROT90.h"
#include "images/oled7seg-9-24bit-19x25-ROT90.h"

class SevenSegmentRender {
public:
    SevenSegmentRender(OLED& oled) : 
        _oled(oled) {};

    enum FontSize_t {
        LARGE = 1,
        SMALL = 2
    };

    enum Rotation_t {
        ROT_0 = 1,
        ROT_90 = 2,
        ROT_180 = 3,
        ROT_270 = 4
    };

    /**
     * x, y - position
     * n    - char as numeric 
     * size - large or small
     * rot  - rotation
     */
    void drawNumeric(const int x, const int y, const uint8_t n, const FontSize_t size=LARGE, const Rotation_t rot=ROT_0);

    static const int BLANK = 10;   // blank image with no number overlay

private:
    void drawSmallNumeric(const int x, const int y, const uint8_t n);
    void drawLargeNumeric(const int x, const int y, const uint8_t n);
    void drawSmallNumericRotate90(const int x, const int y, const uint8_t n);

    OLED _oled;
};

#endif
