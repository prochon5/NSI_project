#include <mbed.h>
#include "Si7021.h"
#include "ICM20648.h"
#include <string>


#define READ_INTERVAL 5.0

UnbufferedSerial serial_port(USBTX, USBRX, 115200);
Ticker ticker;

DigitalOut ICM_enable(PF8, 1);
DigitalOut Temp_enable(PF9, 1);

Si7021* temp = new Si7021(PC4, PC5);
ICM20648* imu = new ICM20648(PC0, PC1, PC2, PC3, PF12);

int32_t temperature;

bool measureEnable = false;
bool fallDetect = false;

static void serial_isr();
static void ticker_isr();

static void serial_isr() 
{
    uint8_t buffer;
    while(serial_port.read(&buffer, 1) == 1)
    {
        serial_port.write(&buffer, 1);
    }
}

static void ticker_isr()
{
    measureEnable = true; 
    //fallDetect = true;
}

int main() {

    serial_port.attach(serial_isr, SerialBase::RxIrq);
    ticker.attach(ticker_isr, READ_INTERVAL);
    imu->open();

    while(1)
    {

        if(measureEnable)
        {
            measureEnable = false;
            //Power up sensors and w8 for 1 ms
            ICM_enable = 1;
            Temp_enable = 1;
            thread_sleep_for(1);

            //Read values
            temp->measure();
            temperature = temp->get_temperature();
            char buff[100] = {0};
            sprintf(buff, "temp:%ld.%03ld\n", temperature/1000, abs(temperature%1000));
            serial_port.write(buff, strlen(buff));

            if(temperature/1000 < 40) 
            {
                serial_port.write("Danger - cold\n", 14);
            }

            //Power down sensors
            //ICM_enable = 0;
            Temp_enable = 0;
        }
        if(fallDetect)
        {
            fallDetect = false;
            serial_port.write("Motion\n", 7);
            char buff[100] = {0};
            imu->measure();
            float gyr_x, gyr_y, gyr_z;
            imu->get_gyroscope(&gyr_x, &gyr_y, &gyr_z);
            
            sprintf(buff, "x:%d, y:%d, z:%d\n", (int)(gyr_x), (int)(gyr_y), (int)(gyr_z));
            serial_port.write(buff, strlen(buff));
            
            if(abs((int)(gyr_x)) > 240 || abs((int)(gyr_y)) > 240 || abs((int)(gyr_z)) > 240) 
            {
                serial_port.write("Danger - fall\n", 14);
            }
           // thread_sleep_for(1500);
        }
    }
}