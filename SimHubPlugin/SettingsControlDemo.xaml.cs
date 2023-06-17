using SimHub.Plugins.OutputPlugins.Dash.GLCDTemplating;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.TextFormatting;

namespace User.PluginSdkDemo
{
    /// <summary>
    /// Logique d'interaction pour SettingsControlDemo.xaml
    /// </summary>
    public partial class SettingsControlDemo : UserControl
    {


        

        public DataPluginDemo Plugin { get; }


        

        public SettingsControlDemo()
        {
            InitializeComponent();

            




            var SerialPortSelectionArray = new List<SerialPortChoice>() {
                new SerialPortChoice("COM1", "COM1"),
                new SerialPortChoice("COM2", "COM2"),
                new SerialPortChoice("COM3", "COM3"),
                new SerialPortChoice("COM4", "COM4"),
                new SerialPortChoice("COM5", "COM5"),
                new SerialPortChoice("COM6", "COM6"),
                new SerialPortChoice("COM7", "COM7"),
                new SerialPortChoice("COM8", "COM8"),
                new SerialPortChoice("COM9", "COM9"),
                new SerialPortChoice("COM10", "COM10"),
                new SerialPortChoice("COM11", "COM11"),
                new SerialPortChoice("COM12", "COM12")
            };

            SerialPortSelection.DataContext = SerialPortSelectionArray;

        }

        public byte[] getBytes(DAP_config_st aux)
        {
            int length = Marshal.SizeOf(aux);
            IntPtr ptr = Marshal.AllocHGlobal(length);
            byte[] myBuffer = new byte[length];

            Marshal.StructureToPtr(aux, ptr, true);
            Marshal.Copy(ptr, myBuffer, 0, length);
            Marshal.FreeHGlobal(ptr);

            return myBuffer;
        }

        public SettingsControlDemo(DataPluginDemo plugin) : this()
        {
            this.Plugin = plugin;
        }


        // Wrapped data getters for plugin/wheel settings
        public int TravelDistance
        {
            get => this.Plugin.TravelDistance;
            set { this.Plugin.TravelDistance = value; }
        }

        public int PedalForce
        {
            get => this.Plugin.PedalMaxForce;
            set { this.Plugin.dap_config_st.maxForce = (byte)value; }
        }

        public int PedalMinPosition
        {
            get => this.Plugin.PedalMinPosition;
            set { this.Plugin.PedalMinPosition = value; }
        }

        public int PedalMaxPosition
        {
            get => this.Plugin.PedalMaxPosition;
            set { this.Plugin.PedalMaxPosition = value; }
        }


        public void ResetPedalPosition_click(object sender, RoutedEventArgs e)
        {
            if (Plugin.serialPortConnected)
            {

                if (Plugin._serialPort.BytesToRead > 0)
                {
                    Plugin._serialPort.DiscardInBuffer();
                }

                Plugin._serialPort.Write("1");
                int myInt = 1;
                byte[] b = BitConverter.GetBytes(myInt);
                Plugin._serialPort.Write(b, 0, 4);
                Plugin._serialPort.Write("\n");
                System.Threading.Thread.Sleep(100);
                try
                {
                    while (Plugin._serialPort.BytesToRead > 0)
                    {
                        string message = Plugin._serialPort.ReadLine();
                    }
                }
                catch (TimeoutException) { }
            }
        }


        public void SendConfigToPedal_click(object sender, RoutedEventArgs e)
        {
            if (Plugin.serialPortConnected)
            {
                // https://stackoverflow.com/questions/17338571/writing-bytes-from-a-struct-into-a-file-with-c-sharp
                int length = sizeof(byte) * 21;
                byte[] newBuffer = new byte[length];
                newBuffer = getBytes(this.Plugin.dap_config_st);
                Plugin._serialPort.Write(newBuffer, 0, newBuffer.Length);
                //Plugin._serialPort.Write("\n");

                System.Threading.Thread.Sleep(100);
                try
                {
                    while (Plugin._serialPort.BytesToRead > 0)
                    {
                        string message = Plugin._serialPort.ReadLine();
                    }
                }
                catch (TimeoutException) { }

                if (Plugin.toogleDebug)
                {
                    Plugin.toogleDebug = false;
                    Plugin.dap_config_st.maxForce = 10;

                }
                else 
                {
                    Plugin.toogleDebug = true;
                    Plugin.dap_config_st.maxForce = 90;
                }
                
            }
        }


        public void ConnectToPedal_click(object sender, RoutedEventArgs e)
        {
            Plugin.serialPortConnected = !Plugin.serialPortConnected;

            if (Plugin._serialPort.IsOpen)
            { 
            }

            if (Plugin.serialPortConnected)
            {
                try
                {
                    this.Plugin.PedalMinPosition = 1;
                    Plugin._serialPort.Open();

                    try
                    {
                        while (Plugin._serialPort.BytesToRead > 0)
                        {
                            string message = Plugin._serialPort.ReadLine();
                        }
                    }
                    catch (TimeoutException) { }

                }
                catch (Exception ex)
                {
                    //DisplayData(MessageType.Error, ex.Message);
                    

                    /*Plugin._serialPort.Dispose();
                    Plugin._serialPort.Open();
                    int tmp = 5;*/

                    this.Plugin.PedalMinPosition = 50; 


                }

            }
            else 
            {
                Plugin._serialPort.Close();
            }

        }


        public void SerialPortSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            //int tmp = 5;
            string tmp = (string)SerialPortSelection.SelectedValue;
            Plugin._serialPort.PortName = tmp;

        }


        public class SerialPortChoice
        {
            public SerialPortChoice(string display, string value)
            {
                Display = display;
                Value = value;
            }

            public string Value { get; set; }
            public string Display { get; set; }
        }


    }
}
