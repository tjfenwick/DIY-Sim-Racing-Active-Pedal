using GameReaderCommon;
using SimHub.Plugins;
using System;
using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Windows.Media;

// https://stackoverflow.com/questions/14344305/best-way-to-structure-class-struct-in-c-sharp
[StructLayout(LayoutKind.Sequential, Pack = 1)]
[Serializable]
public struct DAP_config_st
{
    // structure identification via payload
    public byte payloadType;

    // variable to check if structure at receiver matched version from transmitter
    public byte version;

    // To check if structure is valid
    public byte checkSum;

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
    public byte absAmplitude; // In steps

    // geometric properties of the pedal
    // in mm
    public byte lengthPedal_AC;
    public byte horPos_AB;
    public byte verPos_AB;
    public byte lengthPedal_CB;

}



namespace User.PluginSdkDemo
{
    [PluginDescription("My plugin description")]
    [PluginAuthor("OpenSource")]
    [PluginName("DIY active pedal plugin")]
    public class DataPluginDemo : IPlugin, IDataPlugin, IWPFSettingsV2
    {


        public int TravelDistance = 100;
        public int PedalMinPosition = 100;
        public int PedalMaxPosition = 100;
        public int PedalMaxForce = 100;

        public DAP_config_st dap_config_st;

        public bool toogleDebug = false;

        


        //https://www.c-sharpcorner.com/uploadfile/eclipsed4utoo/communicating-with-serial-port-in-C-Sharp/
        public SerialPort _serialPort = new SerialPort("COM7", 921600, Parity.None, 8, StopBits.One);
        
    

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
        public void DataUpdate(PluginManager pluginManager, ref GameData data)
        {
            // Define the value of our property (declared in init)
            if (data.GameRunning)
            {
                if (data.OldData != null && data.NewData != null)
                {
                    if (data.NewData.ABSActive > 0)
                    {
                        _serialPort.Write("2");
                    }
                    /*else
                    {
                        _serialPort.Write("3");
                    }*/
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
            SimHub.Logging.Current.Info("Starting plugin");

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




            // https://learn.microsoft.com/de-de/dotnet/api/system.io.ports.serialport?view=dotnet-plat-ext-7.0
            Console.WriteLine("Available Ports:");
            foreach (string s in SerialPort.GetPortNames())
            {
                Console.WriteLine("   {0}", s);
            }
            int tmp = 5;

            if (_serialPort.IsOpen)
            {
                _serialPort.Close();
            }


            //_serialPort.Handshake = Handshake.None;
            _serialPort.ReadTimeout = 500;
            _serialPort.WriteTimeout = 500;

            dap_config_st.payloadType = 100;
            dap_config_st.version = 0;
            dap_config_st.pedalStartPosition = 35;
            dap_config_st.pedalEndPosition = 80;
            dap_config_st.maxForce = 10;//90;
            dap_config_st.relativeForce_p000 = 0;
            dap_config_st.relativeForce_p020 = 20;
            dap_config_st.relativeForce_p040 = 40;
            dap_config_st.relativeForce_p060 = 60;
            dap_config_st.relativeForce_p080 = 80;
            dap_config_st.relativeForce_p100 = 100;
            dap_config_st.dampingPress = 0;
            dap_config_st.dampingPull = 0;
            dap_config_st.absFrequency = 5;
            dap_config_st.absAmplitude = 100;
            dap_config_st.lengthPedal_AC = 150;
            dap_config_st.horPos_AB = 215;
            dap_config_st.verPos_AB = 80;
            dap_config_st.lengthPedal_CB = 200;

            /*_serialPort.Open();
            _serialPort.Write("1");
            _serialPort.Close();
            */



        }
    }
}