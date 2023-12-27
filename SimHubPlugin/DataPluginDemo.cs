using GameReaderCommon;
//using log4net.Plugin;
using SimHub.Plugins;
using SimHub.Plugins.OutputPlugins.Dash.GLCDTemplating;
using System;
using System.IO.Ports;
using System.Runtime;
using System.Runtime.InteropServices;
using System.Windows.Media;




// https://stackoverflow.com/questions/14344305/best-way-to-structure-class-struct-in-c-sharp
[StructLayout(LayoutKind.Sequential, Pack = 1)]
[Serializable]





public struct payloadHeader
{
    // structure identification via payload
    public byte payloadType;

    // variable to check if structure at receiver matched version from transmitter
    public byte version;

    public byte storeToEeprom;
}


public struct payloadPedalAction
{
    public byte triggerAbs_u8;
    public byte resetPedalPos_u8;
    public byte startSystemIdentification_u8;
    public byte returnPedalConfig_u8;
};

public struct payloadPedalConfig
{
    // configure pedal start and endpoint
    // In percent
    public byte pedalStartPosition;
    public byte pedalEndPosition;

    // configure pedal forces
    public byte maxForce;
    public byte preloadForce;

    // design force vs travel curve
    // In percent
    public byte relativeForce_p000;
    public byte relativeForce_p020;
    public byte relativeForce_p040;
    public byte relativeForce_p060;
    public byte relativeForce_p080;
    public byte relativeForce_p100;

    // parameter to configure damping
    public byte dampingPress;
    public byte dampingPull;

    // configure ABS effect 
    public byte absFrequency; // In Hz
    public byte absAmplitude; // In kg/20

    

    // geometric properties of the pedal
    // in mm
    public byte lengthPedal_AC;
    public byte horPos_AB;
    public byte verPos_AB;
    public byte lengthPedal_CB;
    public byte Simulate_ABS_trigger; //simulateABS
    public byte Simulate_ABS_value; //simulated ABS value

    // cubic spline params
    public float cubic_spline_param_a_0;
    public float cubic_spline_param_a_1;
    public float cubic_spline_param_a_2;
    public float cubic_spline_param_a_3;
    public float cubic_spline_param_a_4;

    public float cubic_spline_param_b_0;
    public float cubic_spline_param_b_1;
    public float cubic_spline_param_b_2;
    public float cubic_spline_param_b_3;
    public float cubic_spline_param_b_4;

    // PID settings
    public float PID_p_gain;
    public float PID_i_gain;
    public float PID_d_gain;

    public byte control_strategy_b;

    public byte maxGameOutput;

    // Kalman filter model noise
    public byte kf_modelNoise;

    // debug flags, sued to enable debug output
    public byte debug_flags_0;

    // loadcell rating in kg / 2 --> to get value in kg, muiltiply by 2
    public byte loadcell_rating;


}

public struct payloadFooter
{
    // To check if structure is valid
    public UInt16 checkSum;
}



public struct DAP_action_st
{
    public payloadHeader payloadHeader_;
    public payloadPedalAction payloadPedalAction_;
    public payloadFooter payloadFooter_;
}

public struct DAP_config_st
{
    public payloadHeader payloadHeader_;
    public payloadPedalConfig payloadPedalConfig_;
    public payloadFooter payloadFooter_;
}



namespace User.PluginSdkDemo
{
    [PluginDescription("My plugin description")]
    [PluginAuthor("OpenSource")]
    [PluginName("DIY active pedal plugin")]
    public class DataPluginDemo : IPlugin, IDataPlugin, IWPFSettingsV2
    {


        public bool sendAbsSignal = false;
		public DAP_config_st dap_config_initial_st;

        // ABS trigger timer
        DateTime absTrigger_currentTime = DateTime.Now;
        DateTime absTrigger_lastTime = DateTime.Now;





        //https://www.c-sharpcorner.com/uploadfile/eclipsed4utoo/communicating-with-serial-port-in-C-Sharp/
        public SerialPort[] _serialPort = new SerialPort[3] {new SerialPort("COM7", 921600, Parity.None, 8, StopBits.One),
            new SerialPort("COM7", 921600, Parity.None, 8, StopBits.One),
            new SerialPort("COM7", 921600, Parity.None, 8, StopBits.One)};
        
    

        public bool serialPortConnected = false;


        public DataPluginDemoSettings Settings;

        /// <summary>
        /// Instance of the current plugin manager
        /// </summary>
        public PluginManager PluginManager { get; set; }

        /// <summary>
        /// Gets the left menu icon. Icon must be 24x24 and compatible with black and white display.
        /// </summary>
        public ImageSource PictureIcon => this.ToIcon(Properties.Resources.sdkmenuicon);

        /// <summary>
        /// Gets a short plugin title to show in left menu. Return null if you want to use the title as defined in PluginName attribute.
        /// </summary>
        public string LeftMenuTitle => "DIY active pedal plugin";

        /// <summary>
        /// Called one time per game data update, contains all normalized game data,
        /// raw data are intentionnally "hidden" under a generic object type (A plugin SHOULD NOT USE IT)
        ///
        /// This method is on the critical path, it must execute as fast as possible and avoid throwing any error
        ///
        /// </summary>
        /// <param name="pluginManager"></param>
        /// <param name="data">Current game data, including current and previous data frame.</param>
        /// 
        unsafe public UInt16 checksumCalc(byte* data, int length)
        {

            UInt16 curr_crc = 0x0000;
            byte sum1 = (byte)curr_crc;
            byte sum2 = (byte)(curr_crc >> 8);
            int index;
            for (index = 0; index < length; index = index + 1)
            {
                int v = (sum1 + (*data));
                sum1 = (byte)v;
                sum1 = (byte)(v % 255);

                int w = (sum1 + sum2) % 255;
                sum2 = (byte)w;

                data++;// = data++;
            }

            int x = (sum2 << 8) | sum1;
            return (UInt16)x;
        }

        public byte[] getBytes_Action(DAP_action_st aux)
        {
            int length = Marshal.SizeOf(aux);
            IntPtr ptr = Marshal.AllocHGlobal(length);
            byte[] myBuffer = new byte[length];

            Marshal.StructureToPtr(aux, ptr, true);
            Marshal.Copy(ptr, myBuffer, 0, length);
            Marshal.FreeHGlobal(ptr);

            return myBuffer;
        }

        unsafe public void DataUpdate(PluginManager pluginManager, ref GameData data)
        {
			
			bool sendAbsSignal_local_b = false;
            bool sendTcSignal_local_b = false;


            // Send ABS signal when triggered by the game
            if (data.GameRunning)
            {
                if (data.OldData != null && data.NewData != null)
                {
                    if (data.NewData.ABSActive > 0)
                    {
						sendAbsSignal_local_b = true;
                    }

                    if (data.NewData.TCActive > 0)
                    {
                        sendTcSignal_local_b = true;
                    }

                }
            }
			
			// Send ABS test signal if requested
            if (sendAbsSignal)
            {
                sendAbsSignal_local_b = true;
                sendTcSignal_local_b = true;
            }


            absTrigger_currentTime = DateTime.Now;
            TimeSpan diff = absTrigger_currentTime - absTrigger_lastTime;
            int millisceonds = (int)diff.TotalMilliseconds;
            if (millisceonds <= 5)
            {
                sendAbsSignal_local_b = false;
                sendTcSignal_local_b = false;
            }
            else
            {
                absTrigger_lastTime = DateTime.Now;
            }




            // Send ABS trigger signal via serial
            if (sendAbsSignal_local_b)
			{
				if (_serialPort[1].IsOpen)
                {
                    //_serialPort[1].Write("2");

                    // compute checksum
                    DAP_action_st tmp;
                    tmp.payloadPedalAction_.triggerAbs_u8 = 1;


                    DAP_action_st* v = &tmp;
                    byte* p = (byte*)v;
                    tmp.payloadFooter_.checkSum = checksumCalc(p, sizeof(payloadHeader) + sizeof(payloadPedalAction));


                    int length = sizeof(DAP_action_st);
                    byte[] newBuffer = new byte[length];
                    newBuffer = getBytes_Action(tmp);


                    // clear inbuffer 
                    _serialPort[1].DiscardInBuffer();

                    // send query command
                    _serialPort[1].Write(newBuffer, 0, newBuffer.Length);

                }
			}

            // Send TC trigger signal via serial
            if (sendTcSignal_local_b)
            {
                if (_serialPort[2].IsOpen)
                {
                    // compute checksum
                    DAP_action_st tmp;
                    tmp.payloadPedalAction_.triggerAbs_u8 = 1;


                    DAP_action_st* v = &tmp;
                    byte* p = (byte*)v;
                    tmp.payloadFooter_.checkSum = checksumCalc(p, sizeof(payloadHeader) + sizeof(payloadPedalAction));


                    int length = sizeof(DAP_action_st);
                    byte[] newBuffer = new byte[length];
                    newBuffer = getBytes_Action(tmp);


                    // clear inbuffer 
                    _serialPort[2].DiscardInBuffer();

                    // send query command
                    _serialPort[2].Write(newBuffer, 0, newBuffer.Length);
                }
            }


        }
		
		

        /// <summary>
        /// Called at plugin manager stop, close/dispose anything needed here !
        /// Plugins are rebuilt at game change
        /// </summary>
        /// <param name="pluginManager"></param>
        public void End(PluginManager pluginManager)
        {
            //for(uint pedalIndex = 0; pedalIndex<3; pedalIndex++)
            //{
            //    Settings.selectedComPortNames[pedalIndex] = _serialPort[pedalIndex].PortName;


            //    SimHub.Logging.Current.Info("Diy active pedas plugin - Test 2: " + _serialPort[pedalIndex].PortName);


            //}
            
            // Save settings
            this.SaveCommonSettings("GeneralSettings", Settings);
        }

        /// <summary>
        /// Returns the settings control, return null if no settings control is required
        /// </summary>
        /// <param name="pluginManager"></param>
        /// <returns></returns>
        public System.Windows.Controls.Control GetWPFSettingsControl(PluginManager pluginManager)
        {
            return new SettingsControlDemo(this);
        }

        /// <summary>
        /// Called once after plugins startup
        /// Plugins are rebuilt at game change
        /// </summary>
        /// <param name="pluginManager"></param>
        public void Init(PluginManager pluginManager)
        {
            SimHub.Logging.Current.Info("Starting DIY active pedal plugin");

            // Load settings
            Settings = this.ReadCommonSettings<DataPluginDemoSettings>("GeneralSettings", () => new DataPluginDemoSettings());

            // Declare a property available in the property list, this gets evaluated "on demand" (when shown or used in formulas)
            this.AttachDelegate("CurrentDateTime", () => DateTime.Now);

            // Declare an event
            this.AddEvent("SpeedWarning");


            // Declare an action which can be called
            this.AddAction("IncrementSpeedWarning",(a, b) =>
            {
                Settings.SpeedWarningLevel++;
                SimHub.Logging.Current.Info("Speed warning changed");
            });

            // Declare an action which can be called
            this.AddAction("DecrementSpeedWarning", (a, b) =>
            {
                Settings.SpeedWarningLevel--;
            });


            //Settings.selectedJsonIndexLast[0]
            SimHub.Logging.Current.Info("Diy active pedas plugin - Test 1");
            SimHub.Logging.Current.Info("Diy active pedas plugin - COM port: " + Settings.selectedComPortNames[0]);







            // prepare serial port interfaces
            for (uint pedalIdx = 0; pedalIdx<3; pedalIdx++)
			{
				if (_serialPort[pedalIdx].IsOpen)
				{
					_serialPort[pedalIdx].Close();
				}
				
				_serialPort[pedalIdx].Handshake = Handshake.None;
                _serialPort[pedalIdx].Parity = Parity.None;
                //_serialPort[pedalIdx].StopBits = StopBits.None;


                _serialPort[pedalIdx].ReadTimeout = 2000;
				_serialPort[pedalIdx].WriteTimeout = 500;

                try
                {
                    _serialPort[pedalIdx].PortName = Settings.selectedComPortNames[pedalIdx];
                }
                catch (Exception caughtEx)
                {
                }


            }

            //// check if Json config files are present, otherwise create new ones
            //for (uint jsonIndex = 0; jsonIndex < ComboBox_JsonFileSelected.Items.Count; jsonIndex++)
            //{
            //	// which config file is seleced
            //	string currentDirectory = Directory.GetCurrentDirectory();
            //	string dirName = currentDirectory + "\\PluginsData\\Common";
            //	string jsonFileName = ComboBox_JsonFileSelected(ComboBox_JsonFileSelected.Items[jsonIndex]).Text;
            //	string fileName = dirName + "\\" + jsonFileName + ".json";


            //	// Check if file already exists, otherwise create    
            //	if (!File.Exists(fileName))
            //	{
            //		// create default config
            //		// https://stackoverflow.com/questions/3275863/does-net-4-have-a-built-in-json-serializer-deserializer
            //		// https://learn.microsoft.com/en-us/dotnet/framework/wcf/feature-details/how-to-serialize-and-deserialize-json-data?redirectedfrom=MSDN
            //		var stream1 = new MemoryStream();
            //		var ser = new DataContractJsonSerializer(typeof(DAP_config_st));
            //		ser.WriteObject(stream1, dap_config_initial_st);

            //		stream1.Position = 0;
            //		StreamReader sr = new StreamReader(stream1);
            //		string jsonString = sr.ReadToEnd();

            //		System.IO.File.WriteAllText(fileName, jsonString);
            //	}
            //}



            dap_config_initial_st.payloadHeader_.payloadType = 100;
            dap_config_initial_st.payloadHeader_.version = 110;
            dap_config_initial_st.payloadHeader_.storeToEeprom = 0;
            dap_config_initial_st.payloadPedalConfig_.pedalStartPosition = 35;
            dap_config_initial_st.payloadPedalConfig_.pedalEndPosition = 80;
            dap_config_initial_st.payloadPedalConfig_.maxForce = 90;
            dap_config_initial_st.payloadPedalConfig_.relativeForce_p000 = 0;
            dap_config_initial_st.payloadPedalConfig_.relativeForce_p020 = 20;
            dap_config_initial_st.payloadPedalConfig_.relativeForce_p040 = 40;
            dap_config_initial_st.payloadPedalConfig_.relativeForce_p060 = 60;
            dap_config_initial_st.payloadPedalConfig_.relativeForce_p080 = 80;
            dap_config_initial_st.payloadPedalConfig_.relativeForce_p100 = 100;
            dap_config_initial_st.payloadPedalConfig_.dampingPress = 0;
            dap_config_initial_st.payloadPedalConfig_.dampingPull = 0;
            dap_config_initial_st.payloadPedalConfig_.absFrequency = 5;
            dap_config_initial_st.payloadPedalConfig_.absAmplitude = 100;
            dap_config_initial_st.payloadPedalConfig_.lengthPedal_AC = 150;
            dap_config_initial_st.payloadPedalConfig_.horPos_AB = 215;
            dap_config_initial_st.payloadPedalConfig_.verPos_AB = 80;
            dap_config_initial_st.payloadPedalConfig_.lengthPedal_CB = 200;
            dap_config_initial_st.payloadPedalConfig_.Simulate_ABS_trigger = 0;
            dap_config_initial_st.payloadPedalConfig_.maxGameOutput = 100;

            dap_config_initial_st.payloadPedalConfig_.kf_modelNoise = 128;
            dap_config_initial_st.payloadPedalConfig_.debug_flags_0 = 0;

            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_a_0 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_a_1 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_a_2 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_a_3 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_a_4 = 0;

            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_b_0 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_b_1 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_b_2 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_b_3 = 0;
            dap_config_initial_st.payloadPedalConfig_.cubic_spline_param_b_4 = 0;

            dap_config_initial_st.payloadPedalConfig_.PID_p_gain = 0.3f;
            dap_config_initial_st.payloadPedalConfig_.PID_i_gain = 50.0f;
            dap_config_initial_st.payloadPedalConfig_.PID_d_gain = 0.0f;

            dap_config_initial_st.payloadPedalConfig_.control_strategy_b = 0;

            dap_config_initial_st.payloadPedalConfig_.loadcell_rating = 150;


        }
    }
}