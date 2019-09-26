#ifndef __ERR_CODES_H
#define __ERR_CODES_H

// err codes
typedef int RESULT;
#define RES_OK                      0
#define ERR_METHOD_NOT_IMPLEMENTED -1

// I2C for MAX
#define ERR_BAD_TRANSMISSION      -11
#define ERR_BAD_READ              -12
#define ERR_BAD_PARAM             -13

// DS18B
#define ERR_FAILED_TO_READ_TEMP   -20
#define ERR_FAILED_TO_FIND_DEVICE -21

// Ethernet
#define ERR_FAILED_TO_GET_IP_FROM_DHCP -30
#define ERR_NO_ETHERNET_HW             -31
#define ERR_NO_CABLE_DETECTED          -32

// Fanstate
#define ERR_FAN_NOT_OPERATIONAL -40
#define ERR_FAN_TACH            -41



#endif