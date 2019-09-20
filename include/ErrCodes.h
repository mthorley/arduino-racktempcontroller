#ifndef __ERR_CODES_H
#define __ERR_CODES_H

// err codes
typedef int RESULT;
#define RES_OK                     0

// I2C for MAX
#define ERR_BAD_TRANSMISSION      -1
#define ERR_BAD_READ              -2
#define ERR_BAD_PARAM             -3

// DS18B
#define ERR_FAILED_TO_READ_TEMP   -10
#define ERR_FAILED_TO_FIND_DEVICE -11

// Ethernet
#define ERR_FAILED_TO_GET_IP_FROM_DHCP -20
#define ERR_NO_ETHERNET_HW             -21
#define ERR_NO_CABLE_DETECTED          -22

// Fanstate
#define ERR_FAN_NOT_OPERATIONAL -30
#define ERR_FAN_TACH            -31

#endif