#include <system/cmos.hpp>

#include <io.hpp>
#include <interrupts/interrupts.hpp>
#include <memory/heap.hpp>

uint8_t cmosRead(uint8_t cmosRegister)
{
    ioOutByte(CMOS_ADDRESS, cmosRegister);
    return ioInByte(CMOS_DATA);
}

void cmosWrite(uint8_t cmosRegister, uint8_t value)
{
    ioOutByte(CMOS_ADDRESS, cmosRegister);
    ioOutByte(CMOS_DATA, value);
}

int cmosGetUpdateInProgressFlags()
{
    return (cmosRead(0x0A) & 0x80);
}

k_datetime cmosGetDatetime()
{
    int centuryRegister = 0x0;
    unsigned char century;

    while (cmosGetUpdateInProgressFlags())
        ; // Make sure an update isn't in progress

    
    unsigned char second = cmosRead(0x00);
    unsigned char minute = cmosRead(0x02);
    unsigned char hour = cmosRead(0x04);
    unsigned char day = cmosRead(0x07);
    unsigned char month = cmosRead(0x08);
    unsigned char year = cmosRead(0x09);
    if (centuryRegister != 0)
        century = cmosRead(centuryRegister);

    unsigned char lastSecond;
    unsigned char lastMinute;
    unsigned char lastHour;
    unsigned char lastDay;
    unsigned char lastMonth;
    unsigned char lastYear;
    unsigned char lastCentury;
    unsigned char registerB;

    do
    {
        lastSecond = second;
        lastMinute = minute;
        lastHour = hour;
        lastDay = day;
        lastMonth = month;
        lastYear = year;
        lastCentury = century;

        while (cmosGetUpdateInProgressFlags())
            ;

        second = cmosRead(0x00);
        minute = cmosRead(0x02);
        hour = cmosRead(0x04);
        day = cmosRead(0x07);
        month = cmosRead(0x08);
        year = cmosRead(0x09);
        if (centuryRegister != 0)
            century = cmosRead(centuryRegister);
    } while (
        (lastSecond != second) || (lastMinute != minute) || 
        (lastHour != hour) || (lastDay != day) || 
        (lastMonth != month) || (lastYear != year) || 
        (lastCentury != century));

    registerB = cmosRead(0x0B);

     if (!(registerB & 0x04)) {
            second = (second & 0x0F) + ((second / 16) * 10);
            minute = (minute & 0x0F) + ((minute / 16) * 10);
            hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
            day = (day & 0x0F) + ((day / 16) * 10);
            month = (month & 0x0F) + ((month / 16) * 10);
            year = (year & 0x0F) + ((year / 16) * 10);
            if(centuryRegister != 0) {
                  century = (century & 0x0F) + ((century / 16) * 10);
            }
      }

      // Convert 12 hour clock to 24 hour clock if necessary
 
      if (!(registerB & 0x02) && (hour & 0x80)) {
            hour = ((hour & 0x7F) + 12) % 24;
      }
 
    k_datetime *datetime = new k_datetime();
    datetime->second = second;
    datetime->minute = minute;
    datetime->hour = hour;
    datetime->day = day;
    datetime->month = month;
    datetime->year = year;
      
    return *datetime;
}