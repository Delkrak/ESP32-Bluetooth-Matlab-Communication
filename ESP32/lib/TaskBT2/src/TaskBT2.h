#ifndef TaskBT2_h
#define TaskBT2_h

#include <iostream>
#include "BluetoothSerial.h" 
#include <MPU9250.h>
#include <SPI.h>
#include <MAX525.h>
#include <ADS8688.h>


using namespace std; 

class TaskBT2
{
    public:
        MPU9250FIFO _IMU1, _IMU2;
        MAX525 _DAC;
        ADS8688 _ADC;
                
        TaskBT2(uint8_t CS_IMU1, uint8_t CS_IMU2, uint8_t CS_DAC, uint8_t CS_ADC)
            :_IMU1(SPI, CS_IMU1), _IMU2(SPI, CS_IMU2)   // Constructors and member initializer lists
            ,_DAC(SPI,CS_DAC)
            ,_ADC(CS_ADC){} 
        
        void BeginIMU();
        void BeginDAC();
        void BeginADC();

        void SetTask(uint8_t ValueBT);
        bool RunTask();
        void ExecuteTask();
        void TestTask();

        void SetDACVoltaget(uint8_t Channel, float Voltage);

        std::vector<float> GetSensorDataSerial();
        std::vector<uint8_t> GetSensorDataBT();

    protected:
        
    private:
        void float2uint8(float FloatToBeConverted);
        char int2char(int Value);

        std::vector<uint8_t> _ReinterpretedValue = {0};
        std::vector<uint8_t> _DataBufferBT;
        std::vector<float> _DataBufferSerial;
        
        uint8_t _ArrayBufferBT[];
        
        std::string _task, _str;
        uint32_t _package_counter = 0;

        bool _run;
};

#endif