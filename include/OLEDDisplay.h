#include "RackTempController.h"
#include <FTOLED.h>
#include <fonts/Arial14.h>
#include <fonts/Arial12.h>
#include <fonts/SystemFont5x7.h>
#include "SevenSegmentRender.h"

/**
 * Wrapper class for Freetronics FTOLED
 */
class OLEDDisplay {

public:
    OLEDDisplay(uint8_t CSPin, uint8_t DCPin, uint8_t ResetPin, uint8_t IRPin) :
        _oled(CSPin, DCPin, ResetPin), _box(_oled), _ssr(_oled), _IRPin(IRPin)
    {
        _box.setForegroundColour(BLUE);
        _usingIRSensor = true;
    };

    OLEDDisplay(uint8_t CSPin, uint8_t DCPin, uint8_t ResetPin) :
        _oled(CSPin, DCPin, ResetPin), _box(_oled), _ssr(_oled)
    {
        _box.setForegroundColour(BLUE);
        _usingIRSensor = false;
    };

    OLEDDisplay(OLED oled) :
        _oled(oled), _box(oled), _ssr(oled)
    {
        _box.setForegroundColour(BLUE);
        _usingIRSensor = false;
    };

    void initialise();

    void render(const String& s);
    void render(RackState_t& rs, const NetworkState_t& ns); // std::map implementation does not support const
    void clearDisplay();
    void displayOff();
    void displayOn();
    void setOrientation(OLED_Orientation orient);   // rotate

protected:
    void internalRender(RackState_t&rs, const NetworkState_t& ns);        
    void drawFloat2_1DP(int x, int y, float f);
    void drawPercentage(int x, int y, uint8_t pc);
    void drawTempErrStates(const Thermos_t& thermos, int x, int y);        
    void drawFanErrStates(const Fans_t& fans, int x, int y);
    void drawNetworkState(const NetworkState_t& ns, int x, int y);        
    uint8_t getPercentageRPM(const FanState_t& fan) const;

private:
    int getDigits(uint16_t n, uint8_t* pBuf) const;
    String getIPAddressv4(IPAddress addr) const;
    
    OLED               _oled;         // FTOLED
    OLED_TextBox       _box;          // text box for initialisation status
    SevenSegmentRender _ssr;          
    SevenSegmentRender::Rotation_t  _rotation;  // 7 seg orientation - derived from OLED orientation

    // IR Sensor related
    bool    _usingIRSensor;
    uint8_t _IRPin;
    bool    _firstDisplay = true;
    int     _l = 0;

    typedef union _IPAddress {
        uint8_t bytes[4];  // IPv4 address
        uint32_t dword;
    } IPAddress_t;
};
