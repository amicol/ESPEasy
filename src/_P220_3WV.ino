#ifdef USES_P220
//#######################################################################################################
//#################################### Plugin 220: Level Control ########################################
//#######################################################################################################

#include "PID_v1.h"

#define PLUGIN_220
#define PLUGIN_ID_220        220
#define PLUGIN_NAME_220       "Regulator-3WV"
#define PLUGIN_VALUENAME1_220 "Circulator"
#define PLUGIN_VALUENAME2_220 "3WV_Pos"
#define PLUGIN_VALUENAME3_220 "3WV_Target"
#define MIN_STEP 2
#define MAX_SERVO 100
#define MAX_3WAY 100
#define INTERVAL_REG 45000      // Interval between 3VW temperature checks
#define INTERVAL_MAIN_REG 900000        // Interval between Ambiant temperature checks

#define main_PID_Kp 40  //main PID coeff
#define main_PID_Ki 40
#define main_PID_Kd 0
#define way3_PID_Kp 1.9 //3 way valve PID coeff
#define way3_PID_Ki 1
#define way3_PID_Kd .2


double Output;
//double input;
double target_3WV;
//PID *way3_PID;
//double main_input;
//PID *main_PID;
double ambient_temp;
double input_temp;
double target;
PID main_PID(&ambient_temp, &target_3WV, &target,main_PID_Kp, main_PID_Ki, main_PID_Kd, DIRECT);
PID way3_PID(&input_temp, &Output, &target_3WV,way3_PID_Kp, way3_PID_Ki, way3_PID_Kd, DIRECT);
unsigned long stopServoAt;
unsigned long lastRegulation;
unsigned long last_main_Regulation;
bool start_circulator;
uint8_t servoPosition;
int step;
bool regulate_on = 1;

// void begin(void)
// {
//
// 	    target_3WV = ambient_temp;
// 			main_input = ambient_temp;
// 			main_PID = new PID (&main_input, &target_3WV, &ambient_temp,main_PID_Kp, main_PID_Ki, main_PID_Kd, DIRECT);
//
// 			Output = 0;
// 			input = input_temp;
// 			way3_PID = new PID (&input, &Output, &target_3WV,way3_PID_Kp, way3_PID_Ki, way3_PID_Kd, DIRECT);
//
// 			//turn the PID on
// 	 		way3_PID.SetMode(AUTOMATIC);
// 			main_PID.SetMode(AUTOMATIC);
//
// 			way3_PID.SetOutputLimits(0, MAX_3WAY);
// 			main_PID.SetOutputLimits(15., PCONFIG_FLOAT(1));
//
//
// 			adjust_PID();
//
//
// }
// void reset(void)
// {
//
// 	//stopServoAt = millis() + TIME_TO_RESET;
// 	digitalWrite(CONFIG_PIN3,HIGH);
// 	digitalWrite(CONFIG_PIN2, LOW);
// 	circulatorOn();
//   stopServoAt = millis() + PCONFIG_FLOAT(0)*1000;
// 	digitalWrite(CONFIG_PIN2,HIGH);
// 	servoPosition = 0;
// 	regulate();
//
// }
void reset_PID(struct EventStruct *event)
{
		start_circulator = true;
		way3_PID.SetMode(MANUAL);
		Output = 0;
		setServoTo(0,event);
		way3_PID.SetMode(AUTOMATIC);
}
void update(struct EventStruct *event)
{

		stopServo(event); // stop the servo if necessary

		if (millis()- lastRegulation >= INTERVAL_REG )// time for regulation if the servo is not running
		{
      String log = F("Regulate Begin :");
      log+=lastRegulation;
      addLog(LOG_LEVEL_INFO, log);
			regulate(event);
		}
		if ((millis() >= last_main_Regulation +INTERVAL_MAIN_REG) )
		{
      String log2 = F("Adjust Begin ");
      addLog(LOG_LEVEL_INFO, log2);
			adjust_PID(event);
		}

}
void adjust_PID(struct EventStruct *event)
{
			//main_input = ambient_temp;
			main_PID.Compute();

			last_main_Regulation = millis();
}
void regulate(struct EventStruct *event)
{
			if (regulate_on == true)
			{
				if (digitalRead(CONFIG_PIN2) != HIGH)
			{
						//input = input_temp;
						way3_PID.Compute();
            String log2 = F("target_3WV :");
            log2+=target_3WV;
            log2+= F("\n Input:");
            log2+= input_temp;
            log2+= F("\n Output :");
            log2+= Output;
            addLog(LOG_LEVEL_INFO, log2);
            UserVar[event->BaseVarIndex + 2] = target_3WV;
						if (abs(servoPosition - Output)>=MIN_STEP)
						{
							setServoTo(int(Output),event);
						}
				}
				// if ( (servoPosition > 5) && (servoPosition < 95) )
				if (servoPosition <= MAX_SERVO)
				 {
				 	circulatorOn(event);
				 }
				else
				{
					circulatorOff(event);
				}
			}
			else
				{
				circulatorOff(event);
				}
			lastRegulation = millis();
}
// void openValve(int num_step)
// {
// 	if ((servoPosition+num_step <= 100) && digitalRead(CONFIG_PIN2) == HIGH && input_temp<PCONFIG_FLOAT(1))
// 	{
// 		stopServoAt = millis() + num_step*step;
// 		digitalWrite(CONFIG_PIN3,LOW);
// 		digitalWrite(CONFIG_PIN2, LOW);
// 		servoPosition = servoPosition + num_step;
//     lastRegulation = millis();
// 	}
// }
// void closeValve(int num_step)
// {
// 	if ((servoPosition - num_step >= 0) && digitalRead(CONFIG_PIN2) == HIGH)
// 	{
// 		stopServoAt = millis() + num_step*step;
// 		digitalWrite(CONFIG_PIN3,HIGH);
// 		digitalWrite(CONFIG_PIN2, LOW);
// 		servoPosition = servoPosition - num_step;
//     lastRegulation = millis();
// 	}
// }
void stopServo(struct EventStruct *event)
{
	if (stopServoAt <= millis() && digitalRead(CONFIG_PIN2) == HIGH)
	{
    String log = F("Stop servo ");
    addLog(LOG_LEVEL_INFO, log);
		digitalWrite(CONFIG_PIN3,LOW);
		digitalWrite(CONFIG_PIN2, LOW);
    UserVar[event->BaseVarIndex + 1] = servoPosition ;
		if ((start_circulator == true))
		{
			circulatorOn(event);
      start_circulator = false;
		}
	}
}
void setServoTo(int requestedPosition, struct EventStruct *event)
{
  String log = F("setServoTo ");
  log+=requestedPosition;
  addLog(LOG_LEVEL_INFO, log);
	if (requestedPosition != servoPosition && digitalRead(CONFIG_PIN2) == LOW )
	{
		if (requestedPosition>95) {requestedPosition=100;}
		if (requestedPosition<5) {requestedPosition=0;}
		int numberOfSteps = requestedPosition - servoPosition;
		if (numberOfSteps > 0) // if the number is positive or higher than current, open the valve and use X steps
		{
			if (requestedPosition == 100) numberOfSteps+=20;
			stopServoAt = millis() + (abs(numberOfSteps) * step);
			digitalWrite(CONFIG_PIN3, HIGH);
			digitalWrite(CONFIG_PIN2, HIGH);

		}
		else if (numberOfSteps < 0)
		{
			if (requestedPosition == 0) numberOfSteps+=20;
			stopServoAt = millis() + (abs(numberOfSteps)* step);
			digitalWrite(CONFIG_PIN3, LOW);
			digitalWrite(CONFIG_PIN2, HIGH);

		}
		servoPosition = requestedPosition;
	}
}
void circulatorOff(struct EventStruct *event)
{
  String log = F("Circulator Off ");
  addLog(LOG_LEVEL_INFO, log);
  UserVar[event->BaseVarIndex] = 0;
	if (digitalRead(CONFIG_PIN1) == HIGH)
	{
		digitalWrite(CONFIG_PIN1, LOW);

	}
}
void circulatorOn(struct EventStruct *event)
{
  String log = F("Circulator On ");
  addLog(LOG_LEVEL_INFO, log);
  UserVar[event->BaseVarIndex] = 1;
	if (digitalRead(CONFIG_PIN1) == LOW)
	{
		digitalWrite(CONFIG_PIN1, HIGH);
	}
}

boolean Plugin_220(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_220;
        Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_220);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_220));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_220));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_220));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Circulator"));
        event->String2 = formatGpioName_output(F("3WV_On_Off"));
        event->String3 = formatGpioName_output(F("3WV_Direction"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // char tmpString[128];

        addHtml(F("<TR><TD>Ambient Task:<TD>"));
        addTaskSelect(F("p220_task_ambient"), PCONFIG(0));

        LoadTaskSettings(PCONFIG(0)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Ambient Value:<TD>"));
        addTaskValueSelect(F("p220_value_ambient"), PCONFIG(1), PCONFIG(0));

        addHtml(F("<TR><TD>Input Task:<TD>"));
        addTaskSelect(F("p220_task_input"), PCONFIG(2));

        LoadTaskSettings(PCONFIG(2)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Input Value:<TD>"));
        addTaskValueSelect(F("p220_value_input"), PCONFIG(3), PCONFIG(2));

        addHtml(F("<TR><TD>Output Task:<TD>"));
        addTaskSelect(F("p220_task_ouput"), PCONFIG(4));

        LoadTaskSettings(PCONFIG(4)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Output Value:<TD>"));
        addTaskValueSelect(F("p220_value_output"), PCONFIG(5), PCONFIG(4));

        addHtml(F("<TR><TD>Set Point Task:<TD>"));
        addTaskSelect(F("p220_task_setpoint"), PCONFIG(6));

        LoadTaskSettings(PCONFIG(6)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Set Point Value:<TD>"));
        addTaskValueSelect(F("p220_value_setpoint"), PCONFIG(7), PCONFIG(6));

        addFormTextBox(F("Time to reset 3WV"), F("p220_resetvalue"), String(PCONFIG_FLOAT(0)), 8);

        addFormTextBox(F("Maximum temperature"), F("p220_tempmax"), String(PCONFIG_FLOAT(1)), 8);

        LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p220_task_ambient"));
        PCONFIG(1) = getFormItemInt(F("p220_value_ambient"));
        PCONFIG(2) = getFormItemInt(F("p220_task_input"));
        PCONFIG(3) = getFormItemInt(F("p220_value_input"));
        PCONFIG(4) = getFormItemInt(F("p220_task_output"));
        PCONFIG(5) = getFormItemInt(F("p220_value_output"));
        PCONFIG(6) = getFormItemInt(F("p220_task_setpoint"));
        PCONFIG(7) = getFormItemInt(F("p220_value_setpoint"));
        PCONFIG_FLOAT(0) = getFormItemFloat(F("p220_resetvalue"));
        PCONFIG_FLOAT(1) = getFormItemFloat(F("p220_tempmax"));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        for (byte x=0; x<3;x++)
        {
          String log = F("Regulator: value ");
          log += x+1;
          log += F(": ");
          log += UserVar[event->BaseVarIndex+x];
          addLog(LOG_LEVEL_INFO,log);
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("setlevel"))
        {
          String value = parseString(string, 2);
          float result=0;
          Calculate(value.c_str(), &result);
          byte TaskIndex3 = PCONFIG(6);
          byte BaseVarIndex3 = TaskIndex3 * VARS_PER_TASK + PCONFIG(7);

          UserVar[BaseVarIndex3] = result;
          SaveSettings();
          success = true;
        }
        if (command == F("getlevel"))
        {
          byte TaskIndex3 = PCONFIG(6);
          byte BaseVarIndex3 = TaskIndex3 * VARS_PER_TASK + PCONFIG(7);
          string = UserVar[BaseVarIndex3];
          SendStatus(event->Source, string);
          success = true;
        }
        break;
      }

    case PLUGIN_INIT:
      {
        String log = F("Init 3WV Begin ");
        addLog(LOG_LEVEL_INFO, log);
        pinMode(CONFIG_PIN1, OUTPUT);
        pinMode(CONFIG_PIN2, OUTPUT);
        pinMode(CONFIG_PIN3, OUTPUT);
        digitalWrite(CONFIG_PIN1, HIGH);
        digitalWrite(CONFIG_PIN2,HIGH);
        digitalWrite(CONFIG_PIN3, HIGH);
        step = PCONFIG_FLOAT(0)*1000/100;
        String log4 = F("Step :");
        log4+=step;
        addLog(LOG_LEVEL_INFO, log4);

        byte TaskIndex = PCONFIG(6);
        byte BaseVarIndex = TaskIndex * VARS_PER_TASK + PCONFIG(7);
        target = UserVar[BaseVarIndex];

        target_3WV = target;
        //main_input = ambient_temp;
        //main_PID = new PID (&ambient_temp, &target_3WV, &target,main_PID_Kp, main_PID_Ki, main_PID_Kd, DIRECT);

        Output = 0;
        //input = input_temp;
        //way3_PID = new PID (&input_temp, &Output, &target_3WV,way3_PID_Kp, way3_PID_Ki, way3_PID_Kd, DIRECT);

        //turn the PID on
        way3_PID.SetMode(AUTOMATIC);
        main_PID.SetMode(AUTOMATIC);

        way3_PID.SetOutputLimits(0, MAX_3WAY);
        main_PID.SetOutputLimits(15., PCONFIG_FLOAT(1));

        //stopServoAt = millis() + TIME_TO_RESET;
        digitalWrite(CONFIG_PIN3,LOW);
        digitalWrite(CONFIG_PIN2, HIGH);
        circulatorOn(event);
        stopServoAt = millis() + PCONFIG_FLOAT(0)*1000;
        //digitalWrite(CONFIG_PIN2,LOW);
        String log3 = F("Stop servo at ");
        log3+=stopServoAt;
        addLog(LOG_LEVEL_INFO, log3);
        servoPosition = 0;
        String log2 = F("Init 3WV End ");
        addLog(LOG_LEVEL_INFO, log2);
        adjust_PID(event);

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        // we're checking a var from another task, so calculate that basevar
        byte TaskIndex = PCONFIG(0);
        byte BaseVarIndex = TaskIndex * VARS_PER_TASK + PCONFIG(1);
        ambient_temp = UserVar[BaseVarIndex];
        
        byte TaskIndex2 = PCONFIG(2);
        byte BaseVarIndex2 = TaskIndex2 * VARS_PER_TASK + PCONFIG(3);
        input_temp = UserVar[BaseVarIndex2];

        byte TaskIndex3 = PCONFIG(6);
        byte BaseVarIndex3 = TaskIndex3 * VARS_PER_TASK + PCONFIG(7);

        if (target != UserVar[BaseVarIndex3])
        {
          String log3 = F("target/UserVar ");
              log3+=target;
              log3+=F(" ");
              log3+=UserVar[BaseVarIndex3];
              addLog(LOG_LEVEL_INFO, log3);

          target = UserVar[BaseVarIndex3];
          adjust_PID(event);
          regulate(event);
        }
        if (target==0)
          {
            regulate_on=0;
          }
        else
          {
            if (regulate_on==0)
            {
              regulate_on=1;
              reset_PID(event);
              adjust_PID(event);
              regulate(event);
            }
        }
        update(event);

        // if (state != switchstate[event->TaskIndex])
        // {
        //   String log = F("LEVEL: State ");
        //   log += state;
        //   addLog(LOG_LEVEL_INFO, log);
        //   switchstate[event->TaskIndex] = state;
        //   digitalWrite(CONFIG_PIN1,state);
        //   UserVar[event->BaseVarIndex] = state;
        //   sendData(event);
        // }

        success = true;
        break;
      }

  }
  return success;
}



#endif // USES_P220
