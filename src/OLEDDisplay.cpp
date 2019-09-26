#include "OLEDDisplay.h"
#include <stdlib.h>

void OLEDDisplay::initialise() {
    _oled.begin();
    _oled.selectFont(Arial14);

    if (_usingIRSensor)
        pinMode(_IRPin, INPUT);    
}

void OLEDDisplay::render(const String& s) {
    _box.println(s);
}

void OLEDDisplay::clearDisplay() {
    _oled.clearScreen();
}

void OLEDDisplay::displayOff() {
    _oled.setDisplayOn(false);
}

void OLEDDisplay::render(RackState_t&rs, const NetworkState_t& ns) {

    if (!_usingIRSensor) {
        // just render if no IR sensor is configured
        internalRender(rs, ns);        
        return;
    }

    // if IR sensor triggered ...
    byte state = digitalRead(_IRPin);    
    if (state == 1 || _firstDisplay) {
        internalRender(rs, ns);
    }
    else {
        // leave on for a couple of loops
        if (_l<4) {
            _l++;
        }
        else {
            displayOff();
            _l = 0;            
        }
    }
}

void OLEDDisplay::internalRender(RackState_t&rs, const NetworkState_t& ns) {

    if (_firstDisplay) {
        clearDisplay();
        _firstDisplay = false;
    }

    _oled.setDisplayOn(true);
    _oled.selectFont(Arial14);

    int x = 0;
    int y = 128 - 25 - 18;

    // FIXME: Make this const of the class
    OLED_Colour LINE_COL = DARKGRAY;

    _oled.drawString(x, 128-18, F("TEMP C"), WHITE, BLACK);
    _oled.drawString(x, y-1-18, F("RPM %"), WHITE, BLACK);
    _oled.drawCircle(38, 128-4, 2, WHITE);  // degree symbol
    _oled.drawLine(x+65, 0, x+65, 128, LINE_COL);
    _oled.drawLine(0, y-2, 128, y-2, LINE_COL);
    
    // draw temps
    float f = rs.thermos["topRack"].tempCelsuis;
    drawFloat2_1DP(x, y, f);
    
    f = rs.thermos["baseRack"].tempCelsuis;
    drawFloat2_1DP(x+66, y, f);

    // draw temp errs
    drawTempErrStates(rs.thermos, x+68, 128-16);

    // draw tachs
    y = y - 1 - 18;

    uint8_t pc = 
    getPercentageRPM(rs.fans[1]);
    drawPercentage(x, y-25, pc);

    pc = getPercentageRPM(rs.fans[2]);
    drawPercentage(x, y-52, pc);

    pc = getPercentageRPM(rs.fans[3]);
    drawPercentage(x+66, y-25, pc);

    pc = getPercentageRPM(rs.fans[4]);
    drawPercentage(x+66, y-52, pc);

    _oled.drawLine(0, y-54, 128, y-54, LINE_COL);
    
    // draw error states: fan errors
    x = 68;
    y = y + 3;
    drawFanErrStates(rs.fans, x, y);

    // network state
    drawNetworkState(ns, 0, 0);
}

// TODO: ethernet connection lost from networkstate
// TODO: mqtt lost from networkstate
void OLEDDisplay::drawNetworkState(const NetworkState_t& ns, int x, int y) {
    _oled.selectFont(Arial12);
    _oled.drawString(x, y, getIPAddressv4(ns.ethernetIP), WHITE, BLACK);
}

void OLEDDisplay::drawTempErrStates(const Thermos_t& thermos, int x, int y) {

    _oled.selectFont(System5x7);
    int w = 12;
    for (auto it = thermos.begin(); it != thermos.end(); it++) {
        if (it->second.result != RES_OK) {
            _oled.drawFilledBox(x, y, x+w, y+10, RED);
            _oled.drawString(x+4, y+2, (it->first == "topRack") ? "T" : "B", BLACK, RED);
        }
        else {
            // clear
            _oled.drawFilledBox(x, y, x+w, y+10, BLACK);            
        }
        x += w + 3;
    }
}

void OLEDDisplay::drawFanErrStates(const Fans_t& fans, int x, int y) {
    
    _oled.selectFont(System5x7);
    int w = 12;
    for (auto it = fans.begin(); it != fans.end(); it++) {
        if (it->second.result != RES_OK) {
            _oled.drawFilledBox(x, y, x+w, y+10, RED);
            _oled.drawString(x+1, y+2, it->second.position.c_str(), BLACK, RED);
        }
        else {
            // clear
            _oled.drawFilledBox(x, y, x+w, y+10, BLACK);            
        }
        x += w + 3;
    }
}

uint8_t OLEDDisplay::getPercentageRPM(const FanState_t& fan) const {
    if (fan.rpm < fan.minRpm)
        return 0;
    else if (fan.rpm > fan.maxRpm)  // maybe due to "noise" on tach pin
        return 100;
    else 
        //return round(((float)(fan.rpm - fan.minRpm) / (fan.maxRpm - fan.minRpm)) * 100.0);
        return round( ((float)fan.rpm / fan.maxRpm) * 100 );
}

void OLEDDisplay::drawPercentage(int x, int y, uint8_t pc) {
    if (pc<0 || pc>100) {
        Log.error(F("Number out of bounds - %d"), pc);
        return; // ERR_OUT_OF_BOUNDS
    }        

    uint8_t buf[3];
    getDigits(pc, &buf[0]);

    // dont display leading zeros
    (buf[0] == 0) ? 
        _ssr.drawNumeric(x, y, SevenSegmentRender::BLANK, SevenSegmentRender::SMALL) :
        _ssr.drawNumeric(x, y, buf[0], SevenSegmentRender::SMALL);

    (buf[0] == 0 && buf[1] == 0) ? 
        _ssr.drawNumeric(x+20, y, SevenSegmentRender::BLANK, SevenSegmentRender::SMALL) :
        _ssr.drawNumeric(x+20, y, buf[1], SevenSegmentRender::SMALL);
    
    _ssr.drawNumeric(x+40, y, buf[2], SevenSegmentRender::SMALL);
}

void OLEDDisplay::drawFloat2_1DP(int x, int y, float f) {

    if (f<0 || f>99.9) {
        Log.error(F("Number out of bounds - %f"), f);
        return; // ERR_OUT_OF_BOUNDS
    }        

    uint16_t n = f * 10;    // scale float with 1DP to int

    uint8_t buf[3];
    getDigits(n, &buf[0]);

    (buf[0] == 0) ? 
        _ssr.drawNumeric(x, y, SevenSegmentRender::BLANK, SevenSegmentRender::SMALL) :
        _ssr.drawNumeric(x, y, buf[0], SevenSegmentRender::SMALL);

    _ssr.drawNumeric(x+20, y, buf[1], SevenSegmentRender::SMALL);
    _oled.drawFilledCircle(x+41, y+2, 1, WHITE);    // decimal point
    _ssr.drawNumeric(x+44, y, buf[2], SevenSegmentRender::SMALL);
}

/**
 * Converts uint16_t into individual 3 chars of uint8_t
 */
int OLEDDisplay::getDigits(uint16_t n, uint8_t* pBuf) const {
    
    if (n<0 || n>999)
        return -1; // ERR_OUT_OF_BOUNDS

    uint8_t i = n / 100 % 10;
    pBuf[0] = i;
    i = n / 10 % 10;
    pBuf[1] = i;
    i = n % 10;
    pBuf[2] = i;

    return 0;
}

String OLEDDisplay::getIPAddressv4(IPAddress addr) const {
    String s;
    IPAddress_t raw;
    raw.dword = (uint32_t)addr;
    for(int i=0;i<4;i++) {
        s = s + raw.bytes[i];
        if (i<3)
            s = s + ".";
    }
    return s;
}
