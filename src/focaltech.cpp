/////////////////////////////////////////////////////////////////
/*
MIT License

Copyright (c) 2019 lewis he

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

focaltech.cpp - Arduino library for focaltech chip.
Created by Lewis on April 17, 2019.
github:https://github.com/lewisxhe/FT5206_Library
*/
/////////////////////////////////////////////////////////////////
#include "focaltech.h"


#define FT5206_VENDID                   (0x11)
#define FT6206_CHIPID                   (0x06)
#define FT6236_CHIPID                   (0x36)
#define FT6236U_CHIPID                  (0x64)
#define FT5206U_CHIPID                  (0x64)

#define FOCALTECH_REGISTER_MODE         (0x00)
#define FOCALTECH_REGISTER_GEST         (0x01)
#define FOCALTECH_REGISTER_STATUS       (0x02)
#define FOCALTECH_REGISTER_TOUCH1_XH    (0x03)
#define FOCALTECH_REGISTER_TOUCH1_XL    (0x04)
#define FOCALTECH_REGISTER_TOUCH1_YH    (0x05)
#define FOCALTECH_REGISTER_TOUCH1_YL    (0x06)
#define FOCALTECH_REGISTER_THRESHHOLD   (0x80)
#define FOCALTECH_REGISTER_MONITORTIME  (0x87)

#define FOCALTECH_REGISTER_LIB_VERSIONH (0xA1)
#define FOCALTECH_REGISTER_LIB_VERSIONL (0xA2)
#define FOCALTECH_REGISTER_INT_STATUS   (0xA4)
#define FOCALTECH_REGISTER_POWER_MODE   (0x87)
#define FOCALTECH_REGISTER_VENDOR_ID    (0xA3)
#define FOCALTECH_REGISTER_VENDOR1_ID   (0xA8)
#define FOCALTECH_REGISTER_ERROR_STATUS (0xA9)


bool FT5206_Class::probe(void)
{
#ifdef ARDUINO
    _i2cPort->beginTransmission(_address);
    return _i2cPort->endTransmission() == 0;
#endif
    initialization = true;
    return true;
}

bool FT5206_Class::begin(TwoWire &port, uint8_t addr)
{
    _i2cPort = &port;
    _address = addr;
    _readCallbackFunc = nullptr;
    _writeCallbackFunc = nullptr;
    return true;
}

bool FT5206_Class::begin(iic_com_fptr_t read_cb, iic_com_fptr_t write_cb, uint8_t addr)
{
    if (read_cb == nullptr || write_cb == nullptr) {
        return false;
    }
    _readCallbackFunc = read_cb;
    _writeCallbackFunc = write_cb;
    _address = addr;
    return probe();
}

uint8_t FT5206_Class::getDeviceMode(void)
{
    if (!initialization) {
        return 0;
    }
    return readRegister8(FOCALTECH_REGISTER_MODE) & 0x03;
}

GesTrue_t FT5206_Class::getGesture(void)
{
    if (!initialization) {
        return FOCALTECH_NO_GESTRUE;
    }
    uint8_t val = readRegister8(FOCALTECH_REGISTER_GEST);
    switch (val) {
    case 0x10:
        return FOCALTECH_MOVE_UP;
    case 0x14:
        return FOCALTECH_MOVE_LEFT;
    case 0x18:
        return FOCALTECH_MOVE_DOWN;
    case 0x1C:
        return FOCALTECH_MOVE_RIGHT;
    case 0x48:
        return FOCALTECH_ZOOM_IN;
    case 0x49:
        return FOCALTECH_ZOOM_OUT;
    default:
        break;
    }
    return FOCALTECH_NO_GESTRUE;
}

void FT5206_Class::setTheshold(uint8_t value)
{
    if (!initialization) {
        return;
    }
    writeRegister8(FOCALTECH_REGISTER_THRESHHOLD, value);
}

uint8_t FT5206_Class::getThreshold(void)
{
    if (!initialization) {
        return 0;
    }
    return readRegister8(FOCALTECH_REGISTER_THRESHHOLD);
}

uint8_t FT5206_Class::getMonitorTime(void)
{
    if (!initialization) {
        return 0;
    }
    return readRegister8(FOCALTECH_REGISTER_MONITORTIME);
}

void FT5206_Class::setMonitorTime(uint8_t sec)
{
    if (!initialization) {
        return;
    }
    writeRegister8(FOCALTECH_REGISTER_MONITORTIME, sec);
}

void FT5206_Class::enableAutoCalibration(void)
{
    if (!initialization) {
        return;
    }
    writeRegister8(FOCALTECH_REGISTER_MONITORTIME, 0x00);
}

void FT5206_Class::disableAutoCalibration(void)
{
    if (!initialization) {
        return;
    }
    writeRegister8(FOCALTECH_REGISTER_MONITORTIME, 0xFF);
}

void FT5206_Class::getLibraryVersion(uint16_t &version)
{
    uint8_t buffer[2];
    readBytes(FOCALTECH_REGISTER_LIB_VERSIONH, buffer, 2);
    version = (buffer[0] << 8) | buffer[1];
}

void FT5206_Class::enableINT(void)
{
    writeRegister8(FOCALTECH_REGISTER_INT_STATUS, 0);
}

void FT5206_Class::disableINT(void)
{
    writeRegister8(FOCALTECH_REGISTER_INT_STATUS, 1);
}

bool FT5206_Class::getPoint(uint16_t &x, uint16_t &y)
{
    if (!initialization) {
        return false;
    }
    uint8_t buffer[5];
    if (readBytes(FOCALTECH_REGISTER_STATUS, buffer, 5)) {
        if (buffer[0] == 0) {
            return false;
        }
        event = (EventFlag_t)(buffer[1] & 0xC0);
        x = ((buffer[1] << 8) & 0x0F ) | buffer[2];
        y = ((buffer[3] << 8) & 0x0F ) | buffer[4];
        return true;
    }
    return false;
}

uint8_t FT5206_Class::getTouched()
{
    if (!initialization) {
        return 0;
    }
    return readRegister8(FOCALTECH_REGISTER_STATUS);
}

void FT5206_Class::setPowerMode(PowerMode_t m)
{
    if (!initialization) {
        return;
    }
    writeRegister8(FOCALTECH_REGISTER_POWER_MODE, m);
}

uint8_t FT5206_Class::getVendorID(void)
{
    if (!initialization) {
        return 0;
    }
    return readRegister8(FOCALTECH_REGISTER_VENDOR_ID);
}

uint8_t FT5206_Class::getVendor1ID(void)
{
    if (!initialization) {
        return 0;
    }
    return readRegister8(FOCALTECH_REGISTER_VENDOR1_ID);
}

uint8_t FT5206_Class::getErrorCode(void)
{
    if (!initialization) {
        return 0;
    }
    return readRegister8(FOCALTECH_REGISTER_ERROR_STATUS);
}

bool FT5206_Class::getPoint(uint8_t num)
{
    if (!initialization) {
        return false;
    }
    uint16_t _x[2];
    uint16_t _y[2];
    uint16_t _id[2];
    uint8_t _touches = 0;
    uint8_t _data[16];
    readBytes(FOCALTECH_REGISTER_MODE, _data, 16);
    _touches = _data[FOCALTECH_REGISTER_STATUS];
    if ((_touches > 2) || (_touches == 0)) {
        _touches = 0;
        return;
    }
    for (uint8_t i = 0; i < 2; i++) {
        _x[i] = _data[FOCALTECH_REGISTER_TOUCH1_XH + i * 6] & 0x0F;
        _x[i] <<= 8;
        _x[i] |= _data[FOCALTECH_REGISTER_TOUCH1_XL + i * 6];
        _y[i] = _data[FOCALTECH_REGISTER_TOUCH1_YH + i * 6] & 0x0F;
        _y[i] <<= 8;
        _y[i] |= _data[FOCALTECH_REGISTER_TOUCH1_YL + i * 6];
        _id[i] = _data[FOCALTECH_REGISTER_TOUCH1_YH + i * 6] >> 4;
    }
    if ((_touches == 0) || (num > 1)) {
        return false;
    } else {
        return true;
    }
}

uint8_t FT5206_Class::readRegister8(uint8_t reg)
{
    uint8_t value;
    (void)readBytes(reg, &value, 1);
    return value;
}

void FT5206_Class::writeRegister8(uint8_t reg, uint8_t value)
{
    (void)writeBytes(reg, &value, 1);
}

bool FT5206_Class::readBytes( uint8_t reg, uint8_t *data, uint8_t nbytes)
{
    if (_readCallbackFunc != nullptr) {
        return _readCallbackFunc(_address, reg, data, nbytes);
    }
#ifdef ARDUINO
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(reg);
    _i2cPort->endTransmission();
    _i2cPort->requestFrom(_address, nbytes);
    uint8_t index = 0;
    while (_i2cPort->available())
        data[index++] = _i2cPort->read();
#endif
    return nbytes != index;
}

bool FT5206_Class::writeBytes( uint8_t reg, uint8_t *data, uint8_t nbytes)
{
    if (_writeCallbackFunc != nullptr) {
        return _writeCallbackFunc(_address, reg, data, nbytes);
    }
#ifdef ARDUINO
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(reg);
    for (uint8_t i = 0; i < nbytes; i++) {
        _i2cPort->write(data[i]);
    }
    return _i2cPort->endTransmission() != 0;
#endif

}