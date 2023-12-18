using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using static System.Net.Mime.MediaTypeNames;


namespace MQTT_WITH_NET
{
    public partial class Form1 : Form
    {
        string[] data ;
        byte[] Qos;
        MqttClient client;
        public Form1()
        {
            InitializeComponent();
            data = new string[] { "DaoDucPhu" };
            Qos = new byte[] { 0,1,2 };
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            client = new MqttClient("test.mosquitto.org");
            client.Connect("ID", null, null);
            if (client.IsConnected ==  true)
            {
                Console.WriteLine("CONNECTED");
                client.Subscribe(data, Qos);
                Console.WriteLine("Sub OK!");
                client.Publish("DaoDucPhu",Qos);
                Console.WriteLine(data);
            }

        }

        private void EventPublish(object sender, MqttMsgPublishEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (client.IsConnected ==  true)
            {
                client.Subscribe(data, Qos);
                Console.WriteLine("Sub OK!");
            }
        }
    }
}
