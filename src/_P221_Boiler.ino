#ifdef USES_P221
//#######################################################################################################
//#################################### Plugin 221: Level Control ########################################
//#######################################################################################################


#define PLUGIN_221
#define PLUGIN_ID_221        221
#define PLUGIN_NAME_221       "Boiler"
#define PLUGIN_VALUENAME1_221 "Circulator"
#define PLUGIN_VALUENAME2_221 "Temp Boiler"
#define PLUGIN_VALUENAME3_221 "Temp tampon haut"
#define PLUGIN_VALUENAME4_221 "Temp tampon bas"

boolean Plugin_221(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_221;
        Device[deviceCount].Type = SENSOR_TYPE_QUAD;
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
        string = F(PLUGIN_NAME_221);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_221));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_221));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_221));
				strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_221));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Circulator"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // char tmpString[128];

        addHtml(F("<TR><TD>Boiler Task:<TD>"));
        addTaskSelect(F("p221_task_boiler"), PCONFIG(0));

        LoadTaskSettings(PCONFIG(0)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Boiler Value:<TD>"));
        addTaskValueSelect(F("p221_value_boiler"), PCONFIG(1), PCONFIG(0));

        addHtml(F("<TR><TD>High buffer Task:<TD>"));
        addTaskSelect(F("p221_task_high_buffer"), PCONFIG(2));

        LoadTaskSettings(PCONFIG(2)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>High buffer Value:<TD>"));
        addTaskValueSelect(F("p221_value_high_buffer"), PCONFIG(3), PCONFIG(2));

        addHtml(F("<TR><TD>Down buffer Task:<TD>"));
        addTaskSelect(F("p221_task_down_buffer"), PCONFIG(4));

        LoadTaskSettings(PCONFIG(4)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Down buffer Value:<TD>"));
        addTaskValueSelect(F("p221_value_down_buffer"), PCONFIG(5), PCONFIG(4));

        addHtml(F("<TR><TD>Mode Task:<TD>"));
        addTaskSelect(F("p221_task_mode"), PCONFIG(6));

        LoadTaskSettings(PCONFIG(6)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Mode Value:<TD>"));
        addTaskValueSelect(F("p221_value_mode"), PCONFIG(7), PCONFIG(6));

        addFormTextBox(F("Temperature requis"), F("p220_required_temp"), String(PCONFIG_FLOAT(0)), 8);
        addFormTextBox(F("Temperature min"), F("p220_min_temp"), String(PCONFIG_FLOAT(1)), 8);
        addFormTextBox(F("Maximum temperature"), F("p220_max_temp"), String(PCONFIG_FLOAT(2)), 8);

        LoadTaskSettings(event->TaskIndex); // we need to restore our original taskvalues!

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("p221_task_boiler"));
        PCONFIG(1) = getFormItemInt(F("p221_value_boiler"));
        PCONFIG(2) = getFormItemInt(F("p221_task_high_buffer"));
        PCONFIG(3) = getFormItemInt(F("p221_value_high_buffer"));
        PCONFIG(4) = getFormItemInt(F("p221_task_down_buffer"));
        PCONFIG(5) = getFormItemInt(F("p221_value_down_buffer"));
        PCONFIG(6) = getFormItemInt(F("p221_task_mode"));
        PCONFIG(7) = getFormItemInt(F("p221_value_mode"));
        PCONFIG_FLOAT(0) = getFormItemFloat(F("p220_required_temp"));
        PCONFIG_FLOAT(1) = getFormItemFloat(F("p220_min_temp"));
        PCONFIG_FLOAT(2) = getFormItemFloat(F("p220_max_temp"));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        for (byte x=0; x<3;x++)
        {
          String log = F("Boiler: value ");
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
        String log = F("Boiler Begin ");
        addLog(LOG_LEVEL_INFO, log);
        pinMode(CONFIG_PIN1, OUTPUT);
        digitalWrite(CONFIG_PIN1, HIGH);


        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        // we're checking a var from another task, so calculate that basevar
        byte TaskIndex = PCONFIG(0);
        byte BaseVarIndex = TaskIndex * VARS_PER_TASK + PCONFIG(1);
        byte boiler = UserVar[BaseVarIndex];

        TaskIndex = PCONFIG(4);
        BaseVarIndex = TaskIndex * VARS_PER_TASK + PCONFIG(5);
        byte down_buffer = UserVar[BaseVarIndex];

        TaskIndex = PCONFIG(6);
        BaseVarIndex = TaskIndex * VARS_PER_TASK + PCONFIG(7);
        byte mode = UserVar[BaseVarIndex];

        if (mode == 1) //Manual mode
        	{
        		digitalWrite(CONFIG_PIN1, HIGH);
        	}
        else if (mode == 0) //Manual mode
        	{
            digitalWrite(CONFIG_PIN1, LOW);
          }
        else if (boiler>PCONFIG_FLOAT(2)) // circule anyway
        	{
        		digitalWrite(CONFIG_PIN1, HIGH);
        	}
        else if ((boiler>PCONFIG_FLOAT(0)) && (digitalRead(CONFIG_PIN1) == LOW)) // normal mode On
        	{
        		digitalWrite(CONFIG_PIN1, HIGH);
        	}
        else if ((digitalRead(CONFIG_PIN1) == HIGH) && (boiler<PCONFIG_FLOAT(1) || boiler - 10 < down_buffer)) // normal mode Off
        	{
        		digitalWrite(CONFIG_PIN1, LOW);
        	}


        success = true;
        break;
      }

  }
  return success;
}



#endif // USES_P221
