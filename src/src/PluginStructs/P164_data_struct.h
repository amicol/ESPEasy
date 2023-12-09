///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin data structure for P164 "GASES - ENS16x (TVOC, eCO2)"
// Plugin for ENS160 & ENS161 TVOC and eCO2 sensor with I2C interface from ScioSense
// Based upon: https://github.com/sciosense/ENS160_driver
// For documentation see 
// https://www.sciosense.com/wp-content/uploads/documents/SC-001224-DS-9-ENS160-Datasheet.pdf
//
// 2023 By flashmark
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PLUGINSTRUCTS_P164_DATA_STRUCT_H
#define PLUGINSTRUCTS_P164_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P164

#define P164_I2C_ADDR            PCONFIG(0)
#define P164_PCONFIG_TEMP_TASK   PCONFIG(1)
#define P164_PCONFIG_TEMP_VAL    PCONFIG(2)
#define P164_PCONFIG_HUM_TASK    PCONFIG(3)
#define P164_PCONFIG_HUM_VAL     PCONFIG(4)

// 7-bit I2C slave address of the ENS160
#define ENS160_I2CADDR_0        0x52	//ADDR low
#define ENS160_I2CADDR_1        0x53	//ADDR high

struct P164_data_struct : public PluginTaskData_base {
public:

  P164_data_struct(struct EventStruct *event);
  //////P164_data_struct()          = delete;
  virtual ~P164_data_struct() = default;

  bool begin();
  bool read(float& tvoc, float& eco2);
  bool read(float& tvoc, float& eco2, float temp, float hum);
  static bool webformLoad(struct EventStruct *event);
  static bool webformSave(struct EventStruct *event);
  bool tenPerSecond(struct EventStruct *event);
private:

  uint8_t     i2cAddress = ENS160_I2CADDR_0;
  bool        initialized = false;

  bool        evaluateState();                                // Evaluate state machine for next action. Should be called regularly.
	bool        start(uint8_t slaveaddr);                       // Init I2C communication, resets ENS160 and checks its PART_ID. Returns false on I2C problems or wrong PART_ID.
	bool        available()     { return this->_available; }    // Report availability of sensor
	uint8_t     revENS16x()     { return this->_revENS16x; }    // Report version of sensor (0: ENS160, 1: ENS161)
	bool        setMode(uint8_t mode);                          // Set operation mode of sensor

	bool        initCustomMode(uint16_t stepNum);               // Initialize definition of custom mode with <n> steps
	bool        addCustomStep(uint16_t time, bool measureHP0, bool measureHP1, bool measureHP2, bool measureHP3, uint16_t tempHP0, uint16_t tempHP1, uint16_t tempHP2, uint16_t tempHP3);
                                                              // Add a step to custom measurement profile with definition of duration, enabled data acquisition and temperature for each hotplate
																							
	bool        measure();                                      // Perform measurement and stores result in internal variables
	bool        measureRaw();                                   // Perform raw measurement and stores result in internal variables
	bool        set_envdata(float t, float h);                  // Writes t (degC) and h (%rh) to ENV_DATA. Returns "0" if I2C transmission is successful
	bool        set_envdata210(uint16_t t, uint16_t h);         // Writes t and h (in ENS210 format) to ENV_DATA. Returns "0" if I2C transmission is successful
	uint8_t     getMajorRev()   { return this->_fw_ver_major; } // Get major revision number of used firmware
	uint8_t	    getMinorRev()   { return this->_fw_ver_minor; } // Get minor revision number of used firmware
	uint8_t	    getBuild()      { return this->_fw_ver_build; } // Get build revision number of used firmware

	uint8_t	    getAQI()        { return this->_data_aqi; }     // Get AQI value of last measurement 
	uint16_t    getTVOC()       { return this->_data_tvoc; }    // Get TVOC value of last measurement 
	uint16_t    geteCO2()       { return this->_data_eco2; }    // Get eCO2 value of last measurement 
	uint16_t    getAQI500()     { return this->_data_aqi500; }  // Get AQI500 value of last measurement 
	uint32_t    getHP0()        { return this->_hp0_rs; }       // Get resistance of HP0 of last measurement
	uint32_t    getHP1()        { return this->_hp1_rs; }       // Get resistance of HP1 of last measurement
	uint32_t    getHP2()        { return this->_hp2_rs; }       // Get resistance of HP2 of last measurement
	uint32_t    getHP3()        { return this->_hp3_rs; }       // Get resistance of HP3 of last measurement
	uint32_t    getHP0BL()      { return this->_hp0_bl; }       // Get baseline resistance of HP0 of last measurement
	uint32_t    getHP1BL()      { return this->_hp1_bl; }       // Get baseline resistance of HP1 of last measurement
	uint32_t    getHP2BL()      { return this->_hp2_bl; }       // Get baseline resistance of HP2 of last measurement
	uint32_t    getHP3BL()      { return this->_hp3_bl; }       // Get baseline resistance of HP3 of last measurement
	uint8_t     getMISR()       { return this->_misr; }         // Return status code of sensor

  uint8_t     _ADDR = 0;          // ADDR pin number (0: not used)
  uint8_t     _nINT = 0;          // INT pin number (0: not used)
  uint8_t	    _nCS = 0;           // CS pin number (0: not used)

  int         _state = 0;         // General state of the software
  ulong       _lastChange = 0u;   // Timestamp of last state transition
  uint8_t     _opmode = 0;        // Selected ENS160 Mode (as requested by higher level software)
  
  bool        checkPartID();      // Reads the part ID and confirms valid sensor
  bool        clearCommand();     // Initialize idle mode and confirms 
  bool        getFirmware();      // Read firmware revisions
  bool        getStatus();        // Read status register
  bool        writeMode(uint8_t mode);    // Write the opmode register

  bool        _available = false;	// ENS160 available
  uint8_t     _statusReg  = 0;    // ENS160 latest status register readout
  uint8_t     _revENS16x = 0;	    // ENS160 or ENS161 connected? (FW >7)

  uint8_t     _fw_ver_major;
  uint8_t     _fw_ver_minor;
  uint8_t     _fw_ver_build;

  uint16_t    _stepCount;         // Counter for custom sequence

  uint8_t     _data_aqi;
  uint16_t    _data_tvoc;
  uint16_t    _data_eco2;
  uint16_t    _data_aqi500;
  uint32_t    _hp0_rs;
  uint32_t    _hp0_bl;
  uint32_t    _hp1_rs;
  uint32_t    _hp1_bl;
  uint32_t    _hp2_rs;
  uint32_t    _hp2_bl;
  uint32_t    _hp3_rs;
  uint32_t    _hp3_bl;
  uint16_t    _temp;
  int         _slaveaddr;         // Slave address of the ENS160
  uint8_t     _misr;

  //Isotherm, HP0 252°C / HP1 350°C / HP2 250°C / HP3 324°C / measure every 1008ms
  uint8_t _seq_steps[1][8] = { { 0x7C, 0x0A, 0x7E, 0xAF, 0xAF, 0xA2, 0x00, 0x80 }, };

  void            moveToState(int newState);

  static uint8_t  read8(uint8_t addr, byte reg);	
  static bool     read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t num);

  static bool     write8(uint8_t addr, byte reg, byte value);
  static bool     write(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t num);

};
#endif // ifdef USES_P164
#endif // ifndef PLUGINSTRUCTS_P164_DATA_STRUCT_H
