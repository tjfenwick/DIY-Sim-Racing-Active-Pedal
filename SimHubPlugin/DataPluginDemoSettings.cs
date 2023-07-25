namespace User.PluginSdkDemo
{
    /// <summary>
    /// Settings class, make sure it can be correctly serialized using JSON.net
    /// </summary>
    public class DataPluginDemoSettings
    {
        public int SpeedWarningLevel = 100;
		
		public int[] selectedJsonIndexLast = new int[3] {0, 3, 6};

        public string[] selectedComPortNames = {"COM1", "COM1", "COM1"};

        public string[] selectedJsonFileNames = { "1", "2", "3" };


        public int[] selectedComPortNamesInt = new int[3] { -1, -1, -1 };
    }
}