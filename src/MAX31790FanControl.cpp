
#include "MAX31790FanControl.h"
#include "Wire.h"

MAX31790::MAX31790(const uint8_t i2c_address, const uint8_t fans)
    : FanControl(fans)
{
    _deviceAddress = i2c_address;
}

/**
 * Initialise as i2c master with
 * default fanconfig
 */
RESULT MAX31790::initialise()
{
    Wire.begin(); // join bus as master

    // configure all fans
    for (int i = 1; i <= getFanCount(); i++)
    {
        // 7:0 - PWM,
        // 6:5:00 - no spinup
        // 4:0- control fan speed;
        // 3:1 -  TACH input enable
        // 2:0 - tach count
        // 1:0- locked rotor
        // 0:0- PWM control speed
        writeByte(FAN_CONFIG(i), 0x08);
    }

    // mask all fans incl those that are not connected
    writeByte(0x12, 0x3F);
    writeByte(0x13, 0x3F);

    uint8_t b;
    readByte(0x11, b);
    Log.notice(F("Fan fault status 1: %c"), b);

    readByte(0x10, b);
    Log.notice(F("Fan fault status 2: %c"), b);
    return RES_OK;
}

RESULT MAX31790::getGlobalConfiguration(GlobalConfig &config)
{
    uint8_t b;
    RESULT res = readByte(GLOBAL_CONFIG_REG, b);
    if (res != RES_OK)
        return res;

    config.standBy_Not_Run = (b & 0x80);
    config.normal_Not_Reset = (b & 0x40);
    config.not_Bus_Timeout = (b & 0x20);
    config.reserved = 0;
    config.oscillator = static_cast<OscillatorEnum>(b & 0x08);
    config.watchDog = static_cast<I2CWatchDogEnum>(b & 0x06);
    config.watchDogStatus = (b & 0x01);
    return RES_OK;
}

/**
 * Convert %(0-100) dutyCycle into 0-511 range for IC MAX31790.
 */
uint16_t MAX31790::scaleDutyCycle(const uint16_t dutyCycle) const
{
    uint16_t scaled = ((float)dutyCycle / 100) * MAX_DUTY_CYCLE_SCALE;
    return scaled;
}

/** 
 * Write bytes to register and return result
 * dutyCycle range is MIN_DUTY_CYCLE - MAX_DUTY_CYCLE (100%)
 */
RESULT MAX31790::setPWM(const uint8_t fanid, const uint16_t dutyCycle)
{
    ASSERT_RANGE_DUTY_CYCLE(dutyCycle);
    ASSERT_RANGE_FAN_ID(fanid, getFanCount());

    uint8_t buffer[2];

    uint16_t scaled = scaleDutyCycle(dutyCycle);
    uint16_t pwm_bit = scaled << 7;
    buffer[0] = pwm_bit >> 8;
    buffer[1] = pwm_bit;

    return writeBytes(PWMOUT_TARGET_DUTY_CYCLE(fanid), &buffer[0], 2);
}

/**
 * Set PWM for all fans to duty
 */
RESULT MAX31790::setPWMForAll(const uint16_t dutyCycle)
{
    ASSERT_RANGE_DUTY_CYCLE(dutyCycle);
    RESULT res = RES_OK;
    for (int i = 1; i <= getFanCount(); i++)
    {
        res = res & setPWM(i, dutyCycle);
    }
    return res;
}

/**
 * GetTachCount per fan
 */
RESULT MAX31790::getTachCount(const uint8_t fanid, uint16_t &tachCount)
{
    ASSERT_RANGE_FAN_ID(fanid, getFanCount());
    uint8_t buffer[2];
    RESULT res = readBytes(TACH_COUNT(fanid), 2, &buffer[0]);
    if (res != RES_OK)
        return res;

    tachCount = buffer[0];
    tachCount = tachCount << 8;
    tachCount |= buffer[1];
    tachCount = tachCount >> 5;
    return RES_OK;
}

/**
 * Scan all i2c devices between 1 and 127.
 */
void MAX31790::scanForI2C()
{
    Serial.println("Scanning...");
    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.print("i2c device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address);
            Serial.println("  !");
        }
    }
    Serial.println("Scan complete.");
}

/**
 * Read byte from register and return result.
 * RES_OK successful response else ERR
 */
RESULT MAX31790::readByte(const uint8_t address, uint8_t &result)
{
    Wire.beginTransmission(_deviceAddress);
    Wire.write(address);
    if (Wire.endTransmission() != 0)
    {
        Log.error(F("Failed to end transmission"));
        return ERR_BAD_TRANSMISSION;
    }

    uint8_t bytes = 1;
    Wire.requestFrom(_deviceAddress, bytes);

    if (Wire.available() <= 1)
    {
        result = Wire.read(); // Reads the data from the register
        return RES_OK;
    }
    else
    {
        Log.error(F("Failed to read byte for register"));
        return ERR_BAD_READ;
    }
}

/**
 * Read n bytes starting from startAddress into result.
 */
RESULT MAX31790::readBytes(const uint8_t startAddress, const uint8_t n, uint8_t *result)
{
    Wire.beginTransmission(_deviceAddress);
    for (int i = 0; i < n; i++)
    {
        Wire.write(startAddress + i);
        if (Wire.endTransmission() != 0)
        {
            Log.error(F("Failed to end transmission"));
            return ERR_BAD_TRANSMISSION;
        }

        uint8_t q = 1;
        Wire.requestFrom(_deviceAddress, q);

        if (Wire.available() <= 1)
        {
            result[i] = Wire.read(); // Reads the data from the register
        }
        else
        {
            Log.error(F("Failed to read bytes for register"));
            return ERR_BAD_READ;
        }
    }
    return RES_OK;
}

/**
 * Write n bytes to register reg 
 */
RESULT MAX31790::writeBytes(const uint8_t address, const uint8_t *bytes, const uint8_t n)
{
    Wire.beginTransmission(_deviceAddress);
    Wire.write(address);
    for (int i = 0; i < n; i++)
    {
        Wire.write(bytes[i]);
    }
    Wire.endTransmission();
    return RES_OK;
}

/**
 * Write n bytes to register reg 
 */
RESULT MAX31790::writeByte(const uint8_t address, const uint8_t byte)
{
    Wire.beginTransmission(_deviceAddress);
    Wire.write(address);
    Wire.write(byte);
    Wire.endTransmission();
    return RES_OK;
}
