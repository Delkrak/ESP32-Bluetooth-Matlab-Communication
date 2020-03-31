#include "Arduino.h"
#include "TaskBT2.h"
#include <iostream>
#include <vector>

//////////////////////////////////////////////////////////
            ////////// Private //////////
//////////////////////////////////////////////////////////


/* reinterprets the received bluetooth integer value to a char */
char TaskBT2::int2char(int Value){  
    int* p = new int(Value); 
    char* ch = reinterpret_cast<char*>(p); 
    return *ch;
}

/* reinterprets float to four byte uint8_t for BT communication */
void TaskBT2::float2uint8(float FloatToBeConverted){
    uint8_t *_array;                                                                  // uint8_t pointer
    _array = reinterpret_cast<uint8_t*>(&FloatToBeConverted);                         // reinterpret float to four uint8_t 
    _ReinterpretedValue.erase(_ReinterpretedValue.begin(),_ReinterpretedValue.end()); // Erase reinterpret buffer
    for (size_t i = 0; i < 4; i++){
        _ReinterpretedValue.push_back(_array[i]);                                     // store reinterpreted data   
    }
}

//////////////////////////////////////////////////////////
            ////////// Public //////////
//////////////////////////////////////////////////////////

/* Initialixe the ADCs*/
void TaskBT2::BeginADC(){
    Serial.println("Initialize ADC");
    //_ADC.Begin();
}

/* Initialize the IMUs */
void TaskBT2::BeginIMU(){
    Serial.println("Initialize IMU");
    //Serial.println(_IMU1.begin()); // Initiate IMU 1
    //Serial.println(_IMU2.begin()); // Initiate IMU 2
}

/* Initialize DACs */
void TaskBT2::BeginDAC(){
    Serial.println("Initialize DAC");
    //Serial.println(_DAC.begin_Daisy());
}

void TaskBT2::SetDACVoltaget(uint8_t Channel, float Voltage){
    //_DAC.SetVoltage_Daisy(Channel, Voltage);
    cout << "SetDACVoltaget: " << Channel << "@" << Voltage << endl;
}

/* Receive the task specified by the master */
void TaskBT2::SetTask(uint8_t ValueBT){     
    char c = int2char(ValueBT);                     // Change uint8_t to human readable data 
    _str.push_back(c);                              // Store char in vector (temporary data) 
    std::size_t found = _str.find("//");            // Check for stop character
    if(found != std::string::npos){                 // If stop character found == true
        _task.erase(_task.begin(),_task.end());     // Erase previous task
        _task = _str.substr(0,found);               // Specify new task 
        _str.erase(_str.begin(),_str.end());        // Erase temporary data

        cout << "Task found: " << _task << endl;    // Print new task received
        _run = true;                                // Raise flag to execute specifig task 
    }
}

/* Return true if a task should be executed */
bool TaskBT2::RunTask(){                    
    return _run;                           
}


/* Execute the task specified by the Master */
void TaskBT2::ExecuteTask(){
    
    // Sample IMU data only [IMU1x, IMU1y, IMU1z, IMU2x, IMU2y, IMU2z]
    if(_task.compare("IMU") == 0){ 
        
        _IMU1.readSensor(); // Read IMU 1 data 
        _IMU2.readSensor(); // Read IMU 2 data
        
        _DataBufferBT.erase(_DataBufferBT.begin(),_DataBufferBT.end()); // Clear Bluetooth buffer
        
        float2uint8(_IMU1.getAccelX_mss());
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU1.getAccelY_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU1.getAccelZ_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU1.getGyroX_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU1.getGyroY_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU1.getGyroZ_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU2.getAccelX_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU2.getAccelY_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU2.getAccelZ_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU2.getGyroX_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU2.getGyroY_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
        float2uint8(_IMU2.getGyroZ_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
        _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer

        return;
        }
    
    // Sample FSR data only [FSR1, FSR2, FSR3, FSR4, FSR5, FSR6, FSR7, FSR8]
    if(_task.compare("FSR") == 0){
        _ADC.noOpDaisy();                                               // Sample ADC channels
        _DataBufferBT.erase(_DataBufferBT.begin(),_DataBufferBT.end()); // Clear Bluetooth buffer
        std::vector<float> FSR = _ADC.ReturnADC_FSR();                  // Get FSR data
       
        for (size_t i = 0; i < FSR.size(); i++)
        {   
            float2uint8(FSR[i]);                                                                                 // Reinterprets IMU data float to uint8_t data 
             _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());  // Store reinterpreted data in Bluetooth buffer
        }
        return;
        }


    

    // Sample EMG data only [EMG1, EMG2, EMG3, EMG4]
    if(_task.compare("EMG") == 0){
        _ADC.noOpDaisy();                                               // Sample ADC channels
        _DataBufferBT.erase(_DataBufferBT.begin(),_DataBufferBT.end()); // Clear Bluetooth buffer
        std::vector<float> EMG = _ADC.ReturnADC_EMG();                  // Get EMG data
       
        for (size_t i = 0; i < EMG.size(); i++)
        {   
            float2uint8(EMG[i]);                                                                                 // Reinterprets IMU data float to uint8_t data 
             _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());  // Store reinterpreted data in Bluetooth buffer
        }
        return;
        }

    // Samples EMG, FSR and IMU data [FSR, EMG, IMU]
    if(_task.compare("All") == 0){

        if( true ){
            float time = (float)millis() * 1e-3;
            for(int i = 0; i < 24; i++){
                float frequency = 20 / (float)i;
                float measurement = sin(2.0 * 3.14 * frequency * time);
                float2uint8(measurement);
                _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            }
            return;
        }else{
            _DataBufferBT.erase(_DataBufferBT.begin(),_DataBufferBT.end()); // Clear Bluetooth buffer
            _ADC.noOpDaisy();                                               // Sample ADC channels
            _IMU1.readSensor();                                             // Read IMU 1 data 
            _IMU2.readSensor();                                             // Read IMU 2 data

            /*FSR*/
            std::vector<float> FSR = _ADC.ReturnADC_FSR();                  // Get FSR data
            for (size_t i = 0; i < FSR.size(); i++){   
                float2uint8(FSR[i]);                                                                                 // Reinterprets IMU data float to uint8_t data 
                _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());  // Store reinterpreted data in Bluetooth buffer
            }

            /*EMG*/
            std::vector<float> EMG = _ADC.ReturnADC_EMG();                  // Get EMG data
            for (size_t i = 0; i < EMG.size(); i++){   
                float2uint8(EMG[i]);                                                                                 // Reinterprets IMU data float to uint8_t data 
                _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());  // Store reinterpreted data in Bluetooth buffer
            }

            /*IMU*/
            float2uint8(_IMU1.getAccelX_mss());
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU1.getAccelY_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU1.getAccelZ_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU1.getGyroX_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU1.getGyroY_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU1.getGyroZ_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU2.getAccelX_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU2.getAccelY_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU2.getAccelZ_mss());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU2.getGyroX_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU2.getGyroY_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer
            float2uint8(_IMU2.getGyroZ_rads());                                                                  // Reinterprets IMU data float to uint8_t data 
            _DataBufferBT.insert(_DataBufferBT.end(), _ReinterpretedValue.begin(), _ReinterpretedValue.end());   // Store reinterpreted data in Bluetooth buffer

        }
        return;
    }

    // Stops data collection 
    if(_task.compare("Stop") == 0){_run = false; return;}

    /* Calibrate DAC channels for maximum contraction/flexion - FSR
       Use this command when doing a maximum contraction. The gain will
       be adjustet such the FSR output is amplified as much as possible.*/
    if(_task.compare("CalibrateFSR") == 0){

        std::vector<float> FSR;                                                     // FSR buffer reading
        std::vector<float> MeanFSR;                                                 // FSR mean value 
        std::vector<float> Resistance_FSR;                                          // FSR resistance 
        std::vector<float> CalibratedVoltageDAC;                                    // Calibrated voltage for DAC output
        float VoltageOutDAC = 9;                                                    // Maximum voltage when flexing with additional buffer of 1 V   

        for (size_t i = 0; i < 8; i++){_DAC.SetVoltage_Daisy(i, 0.5); MeanFSR.push_back(0); delay(10);}   // Set all DAC channels to 0.5 V and initialize MeanFSR as a zero vector
        
        for (size_t i = 0; i < 500; i++){
            _ADC.noOpDaisy();                                        // Sample ADC channels
            FSR = _ADC.ReturnADC_FSR();                              // Get FSR data

            for (size_t k = 0; k < 8; k++){MeanFSR[k] = FSR[k] + MeanFSR[k];}
            delay(1);
        }

        for (size_t i = 0; i < 8; i++){MeanFSR[i] = MeanFSR[i]/500;}   // Calculate mean - division done here due to numerical issues

        for (size_t i = 0; i < 8; i++){
            Resistance_FSR.push_back( (50000)/(MeanFSR[i]-0.5) );  // Derived from non-inverting feedback law  Vo/Vin = 1 + R2/R1, where R2 = 100k, Vo = FSR[i], Vin ~ 0.5 V
            CalibratedVoltageDAC.push_back( (VoltageOutDAC * Resistance_FSR[i])/(Resistance_FSR[i] + 100000) );  // Find input voltage to DAC Vin = Vo*R1/R1+R2
        }

        for (size_t i = 0; i < 8; i++){_DAC.SetVoltage_Daisy(i, CalibratedVoltageDAC[i]); delay(10);}    // Set all DAC to calibrate value
        
        _task = "Stop";
        return;
    }

    cout << "The task did not exsist" << endl;
    _run = false;
    return;
}

/* Return DataBuffer if Serial communication is used */
std::vector<float> TaskBT2::GetSensorDataSerial(){
        _IMU1.readSensor(); // Read IMU 1 data 
        _IMU2.readSensor(); // Read IMU 2 data
        _DataBufferSerial.erase(_DataBufferSerial.begin(),_DataBufferSerial.end());
        _DataBufferSerial.push_back(_IMU1.getAccelX_mss());           // Get IMU1 XYZ
        _DataBufferSerial.push_back(_IMU1.getAccelY_mss());
        _DataBufferSerial.push_back(_IMU1.getAccelZ_mss());
        _DataBufferSerial.push_back(_IMU2.getAccelX_mss());           // Get IMU2 XYZ
        _DataBufferSerial.push_back(_IMU2.getAccelY_mss());
        _DataBufferSerial.push_back(_IMU2.getAccelZ_mss());
    return _DataBufferSerial;
}

/* Return data for BT communication */
std::vector<uint8_t> TaskBT2::GetSensorDataBT(){
    return _DataBufferBT;
}

/* Set task for testing on MPU side */ 
void TaskBT2::TestTask(){
    _task = "All";
    // _task = "IMU";
    // _task = "EMG";
    // _task = "FSR";
}


