using SimHub.Plugins.OutputPlugins.Dash.GLCDTemplating;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.TextFormatting;
using System.Text.Json;
using FMOD;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.IO;
using System.Text;
using System.Web;
using MahApps.Metro.Controls;
using System.Runtime.CompilerServices;
using System.CodeDom.Compiler;

namespace User.PluginSdkDemo
{
    /// <summary>
    /// Logique d'interaction pour SettingsControlDemo.xaml
    /// </summary>
    public partial class SettingsControlDemo : UserControl
    {


        

        public DataPluginDemo Plugin { get; }

        public DAP_config_st dap_config_st;
        






        public SettingsControlDemo()
        {
            

            dap_config_st.payloadHeader_.payloadType = 100;
            dap_config_st.payloadHeader_.version = 0;

            dap_config_st.payloadPedalConfig_.pedalStartPosition = 35;
            dap_config_st.payloadPedalConfig_.pedalEndPosition = 80;
            dap_config_st.payloadPedalConfig_.maxForce = 90;//90;
            dap_config_st.payloadPedalConfig_.relativeForce_p000 = 0;
            dap_config_st.payloadPedalConfig_.relativeForce_p020 = 20;
            dap_config_st.payloadPedalConfig_.relativeForce_p040 = 40;
            dap_config_st.payloadPedalConfig_.relativeForce_p060 = 60;
            dap_config_st.payloadPedalConfig_.relativeForce_p080 = 80;
            dap_config_st.payloadPedalConfig_.relativeForce_p100 = 100;
            dap_config_st.payloadPedalConfig_.dampingPress = 0;
            dap_config_st.payloadPedalConfig_.dampingPull = 0;
            dap_config_st.payloadPedalConfig_.absFrequency = 5;
            dap_config_st.payloadPedalConfig_.absAmplitude = 100;
            dap_config_st.payloadPedalConfig_.lengthPedal_AC = 150;
            dap_config_st.payloadPedalConfig_.horPos_AB = 215;
            dap_config_st.payloadPedalConfig_.verPos_AB = 80;
            dap_config_st.payloadPedalConfig_.lengthPedal_CB = 200;

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

        public byte[] getBytesPayload(payloadPedalConfig aux)
        {
            int length = Marshal.SizeOf(aux);
            IntPtr ptr = Marshal.AllocHGlobal(length);
            byte[] myBuffer = new byte[length];

            Marshal.StructureToPtr(aux, ptr, true);
            Marshal.Copy(ptr, myBuffer, 0, length);
            Marshal.FreeHGlobal(ptr);

            return myBuffer;
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
            set { this.dap_config_st.payloadPedalConfig_.maxForce = (byte)value; }
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


        private void Slider_PedalMinForce(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.preloadForce = Convert.ToByte(e.NewValue);

            if (dap_config_st.payloadPedalConfig_.preloadForce > dap_config_st.payloadPedalConfig_.maxForce)
            {
                dap_config_st.payloadPedalConfig_.preloadForce = dap_config_st.payloadPedalConfig_.maxForce;
            }
        }

        private void Slider_PedalMaxForce(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.maxForce = Convert.ToByte(e.NewValue);

            if (dap_config_st.payloadPedalConfig_.maxForce < dap_config_st.payloadPedalConfig_.preloadForce)
            {
                dap_config_st.payloadPedalConfig_.maxForce = dap_config_st.payloadPedalConfig_.preloadForce;
            }
        }


        private void Slider_PedalMinPos(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.pedalStartPosition = Convert.ToByte(e.NewValue);

            if (dap_config_st.payloadPedalConfig_.pedalStartPosition > dap_config_st.payloadPedalConfig_.pedalEndPosition)
            {
                dap_config_st.payloadPedalConfig_.pedalStartPosition = dap_config_st.payloadPedalConfig_.pedalEndPosition;
            }
        }

        private void Slider_PedalMaxPos(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.pedalEndPosition = Convert.ToByte(e.NewValue);

            if (dap_config_st.payloadPedalConfig_.pedalEndPosition < dap_config_st.payloadPedalConfig_.pedalStartPosition)
            {
                dap_config_st.payloadPedalConfig_.pedalEndPosition = dap_config_st.payloadPedalConfig_.pedalStartPosition;
            }
        }
		
		private void Slider_AbsAmplitude(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.absAmplitude = Convert.ToByte(e.NewValue);
        }

        private void Slider_AbsFrequency(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.absFrequency = Convert.ToByte(e.NewValue);
        }

        public void TestAbs_click(object sender, RoutedEventArgs e)
        {
            Plugin.sendAbsSignal = !Plugin.sendAbsSignal;
        }

        public void Slider_Dampening(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.dampingPress = Convert.ToByte(e.NewValue);
            dap_config_st.payloadPedalConfig_.dampingPull = Convert.ToByte(e.NewValue);
        }



        private void Update_BrakeForceCurve()
        {
            for (int pointIdx = 0; pointIdx < 6; pointIdx++)
            {

                // http://www.csharphelper.com/howtos/wpf_let_user_draw_polyline.html
                // https://stackoverflow.com/questions/1267687/how-to-move-all-coordinate-from-a-wpf-polyline-object
                Point p = this.Polyline_BrakeForceCurve.Points[pointIdx];
                //p.Y = e.NewValue;

                double pointPos = 0;
                switch (pointIdx)
                {
                    case 0:
                        pointPos = dap_config_st.payloadPedalConfig_.relativeForce_p000;
                        break;
                    case 1:
                        pointPos = dap_config_st.payloadPedalConfig_.relativeForce_p020;
                        break;
                    case 2:
                        pointPos = dap_config_st.payloadPedalConfig_.relativeForce_p040;
                        break;
                    case 3:
                        pointPos = dap_config_st.payloadPedalConfig_.relativeForce_p060;
                        break;
                    case 4:
                        pointPos = dap_config_st.payloadPedalConfig_.relativeForce_p080;
                        break;
                    case 5:
                        pointPos = dap_config_st.payloadPedalConfig_.relativeForce_p100;
                        break;
                    default:
                        pointPos = 0;
                        break;
                }

                p.Y = pointPos;

                this.Polyline_BrakeForceCurve.Points[pointIdx] = p;// = e.NewValue;}

            }

        }

        public void Slider_Force000(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            try
            {
                dap_config_st.payloadPedalConfig_.relativeForce_p000 = Convert.ToByte(e.NewValue);

            // http://www.csharphelper.com/howtos/wpf_let_user_draw_polyline.html
            // https://stackoverflow.com/questions/1267687/how-to-move-all-coordinate-from-a-wpf-polyline-object
            //Point p = this.Polyline_BrakeForceCurve.Points[0];
            //p.Y = e.NewValue;
            //this.Polyline_BrakeForceCurve.Points[0] = p;// = e.NewValue;

            Update_BrakeForceCurve();

            }
            catch (Exception caughtEx)
            {
                string errorMessage = caughtEx.Message;
                TextBox1.Text = errorMessage;
            }    
        }

        public void Slider_Force020(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            
            dap_config_st.payloadPedalConfig_.relativeForce_p020 = Convert.ToByte(e.NewValue);
            Update_BrakeForceCurve();
        }
        public void Slider_Force040(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.relativeForce_p040 = Convert.ToByte(e.NewValue);
            Update_BrakeForceCurve();
        }
        public void Slider_Force060(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.relativeForce_p060 = Convert.ToByte(e.NewValue);
            Update_BrakeForceCurve();
        }
        public void Slider_Force080(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.relativeForce_p080 = Convert.ToByte(e.NewValue);
            Update_BrakeForceCurve();
        }
        public void Slider_Force100(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            dap_config_st.payloadPedalConfig_.relativeForce_p100 = Convert.ToByte(e.NewValue);
            Update_BrakeForceCurve();
        }






        public void SaveStructToJson_click(object sender, RoutedEventArgs e)
        {
            // https://learn.microsoft.com/en-us/dotnet/standard/serialization/system-text-json/how-to?pivots=dotnet-8-0


            try
            {
                // which config file is seleced
                string dirName = "C:\\Program Files (x86)\\SimHub\\PluginsData\\Common";
                string jsonFileName = ComboBox_JsonFileSelected.Text;
                string fileName = dirName + "\\" + jsonFileName + ".json";

                // https://stackoverflow.com/questions/3275863/does-net-4-have-a-built-in-json-serializer-deserializer
                // https://learn.microsoft.com/en-us/dotnet/framework/wcf/feature-details/how-to-serialize-and-deserialize-json-data?redirectedfrom=MSDN
                var stream1 = new MemoryStream();
                var ser = new DataContractJsonSerializer(typeof(DAP_config_st));
                ser.WriteObject(stream1, dap_config_st);



                stream1.Position = 0;
                StreamReader sr = new StreamReader(stream1);

                string jsonString = sr.ReadToEnd();



                // Check if file already exists. If yes, delete it.     
                if (File.Exists(fileName))
                {
                    File.Delete(fileName);
                }


                System.IO.File.WriteAllText(fileName, jsonString);
                
                TextBox1.Text = "Config exported!";

            }
            catch (Exception caughtEx)
            {

                string errorMessage = caughtEx.Message;
                TextBox1.Text = errorMessage;
            }

        }

        public void ReadStructFromJson_click(object sender, RoutedEventArgs e)
        {
            try
            {
                // https://learn.microsoft.com/en-us/dotnet/standard/serialization/system-text-json/how-to?pivots=dotnet-8-0
                // https://www.educative.io/answers/how-to-read-a-json-file-in-c-sharp

                string dirName = "C:\\Program Files (x86)\\SimHub\\PluginsData\\Common";
                string jsonFileName = ComboBox_JsonFileSelected.Text;
                string fileName = dirName + "\\" + jsonFileName + ".json";

                string text = System.IO.File.ReadAllText(fileName);

                DataContractJsonSerializer deserializer = new DataContractJsonSerializer(typeof(DAP_config_st));
                var ms = new MemoryStream(Encoding.UTF8.GetBytes(text));
                dap_config_st = (DAP_config_st)deserializer.ReadObject(ms);
                TextBox1.Text = "Config loaded!";

                // update the sliders
                PedalMinForce_Slider.Value = dap_config_st.payloadPedalConfig_.preloadForce;
                PedalMaxForce_Slider.Value = dap_config_st.payloadPedalConfig_.maxForce;

                PedalMinPos_Slider.Value = dap_config_st.payloadPedalConfig_.pedalStartPosition;
                PedalMaxPos_Slider.Value = dap_config_st.payloadPedalConfig_.pedalEndPosition;

                PedalAbsAmplitude_Slider.Value = dap_config_st.payloadPedalConfig_.absAmplitude;
                PedalAbsFrequency_Slider.Value = dap_config_st.payloadPedalConfig_.absFrequency;

                PedalDampening_Slider.Value = dap_config_st.payloadPedalConfig_.dampingPress;

                PedalForceCurve000_Slider.Value = dap_config_st.payloadPedalConfig_.relativeForce_p000;
                PedalForceCurve020_Slider.Value = dap_config_st.payloadPedalConfig_.relativeForce_p020;
                PedalForceCurve040_Slider.Value = dap_config_st.payloadPedalConfig_.relativeForce_p040;
                PedalForceCurve060_Slider.Value = dap_config_st.payloadPedalConfig_.relativeForce_p060;
                PedalForceCurve080_Slider.Value = dap_config_st.payloadPedalConfig_.relativeForce_p080;
                PedalForceCurve100_Slider.Value = dap_config_st.payloadPedalConfig_.relativeForce_p100;

                Update_BrakeForceCurve();

            }
            catch (Exception caughtEx)
            {

                string errorMessage = caughtEx.Message;
                TextBox1.Text = errorMessage;
            }


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


        unsafe private UInt16 checksumCalc(byte* data, int length)
        {

            UInt16 curr_crc = 0x0000;
            byte sum1 = (byte)curr_crc;
            byte sum2 = (byte)(curr_crc >> 8);
            int index;
            for (index=0; index < length; index = index + 1)
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

            //return (sum2, sum1);

        }
        unsafe public void SendConfigToPedal_click(object sender, RoutedEventArgs e)
        {
            if (Plugin.serialPortConnected)
            {

                // compute checksum
                //getBytes(this.dap_config_st.payloadPedalConfig_)
                payloadPedalConfig tmp = this.dap_config_st.payloadPedalConfig_;
                payloadPedalConfig* v = &tmp;
                byte* p = (byte*)v;
                this.dap_config_st.payloadHeader_.checkSum = checksumCalc(p, sizeof(payloadPedalConfig));


                TextBox1.Text = "CRC simhub calc: " + this.dap_config_st.payloadHeader_.checkSum + "    ";



                int length = sizeof(DAP_config_st);
                int val = this.dap_config_st.payloadHeader_.checkSum;
                string msg = "CRC value: " + val.ToString();
                byte[] newBuffer = new byte[length];
                newBuffer = getBytes(this.dap_config_st);
                Plugin._serialPort.Write(newBuffer, 0, newBuffer.Length);
                //Plugin._serialPort.Write("\n");

                System.Threading.Thread.Sleep(100);
                try
                {
                    while (Plugin._serialPort.BytesToRead > 0)
                    {
                        string message = Plugin._serialPort.ReadLine();

                        TextBox1.Text += "      -->   " + message;
                    }
                }
                catch (TimeoutException) { }

   
                
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
