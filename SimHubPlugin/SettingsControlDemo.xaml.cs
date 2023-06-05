using SimHub.Plugins.OutputPlugins.Dash.GLCDTemplating;
using System;
using System.Collections.Generic;
using System.IO.Ports;
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
                new SerialPortChoice("COM11", "COM11")
            };

            SerialPortSelection.DataContext = SerialPortSelectionArray;

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
                Plugin._serialPort.Write("1");
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
