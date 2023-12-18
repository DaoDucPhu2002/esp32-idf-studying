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

namespace MQTT_V2
{
    public partial class Form1 : Form
    {
        public MqttClient mqClient;
        byte LineNotUse;
        bool[] is_not_use_btn_press;
        byte[] DeviceID;
        byte Rssi;
        byte Status;
        string[] topic_public = new string[1];
        byte[] LineStatus;
        public Form1()
        {
            InitializeComponent();
            topic_public[0] = "Alam_seting";
            DeviceID = new byte[6];
            LineStatus = new byte[8];
            is_not_use_btn_press = new bool[8];
            for (int i = 0; i < 8; i++)
            {
                is_not_use_btn_press[i] = true;
            }
            mqClient = new MqttClient("test.mosquitto.org"); // MQTT Broker
            mqClient.Connect("");
            if (mqClient.IsConnected ==  true)
            {
                mqClient.MqttMsgPublishReceived += MqClient_MqttMsgPublishRec;
                string[] topics = new string[1];
                topics[0] = "Alam_noti";
                byte[] msg = new byte[1];
                msg[0] = MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE;
                
                mqClient.Subscribe(topics, msg);
            }
        }

        static Color CheckColor(byte input)
        { 
            switch (input)
            {
                case 0:
                    return Color.Black; 
                case 1:
                    return Color.Red;
                case 2:
                    return Color.Yellow;
                case 3:
                    return Color.Green;
                   default: return Color.White;
            }
        
        
        }

        private void MqClient_MqttMsgPublishRec(object sender, MqttMsgPublishEventArgs e)
        {
          string topic = e.Topic;
           
            byte[] byteArray = e.Message;
           // string hex_string = BitConverter.ToString(byteArray).Replace("-","");
            if (topic == "Alam_noti")
            {
                Array.Copy(byteArray, 0, DeviceID, 0,6);
                Rssi = byteArray[6];
                LineNotUse = byteArray[7];
                Status = byteArray[8];
                Array.Copy(byteArray,9,LineStatus, 0, 8);
                string DeviceID_string = BitConverter.ToString(DeviceID);
                label_DevieceID.Invoke(new Action(() => label_DevieceID.Text = DeviceID_string));

                label_Rssi.Invoke(new Action(() => label_Rssi.Text = Rssi.ToString("00")));
                button1.BackColor = CheckColor(LineStatus[0]);
                button2.BackColor = CheckColor(LineStatus[1]);
                button3.BackColor = CheckColor(LineStatus[2]);
                button4.BackColor = CheckColor(LineStatus[3]);
                button5.BackColor = CheckColor(LineStatus[4]);
                button6.BackColor = CheckColor(LineStatus[5]);
                button7.BackColor = CheckColor(LineStatus[6]);
                button8.BackColor = CheckColor(LineStatus[7]);
               


            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void not_use_1_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[0] = !is_not_use_btn_press[0];
            if (is_not_use_btn_press[0] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0xfe);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x01);
            }
            this.Invoke(new EventHandler(HienThi));
        }
        void PublishMessage(string topic, byte byteValue)
        {
            mqClient.Publish(topic, new byte[] { byteValue }, MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, false);
        
    }

        private void HienThi(object sender, EventArgs e)
        {
        

            not_use_1.BackColor = (((LineNotUse << 7) & 0x80) == 0) ? Color.Red : Color.Green;
            not_use_2.BackColor = (((LineNotUse << 6) & 0x80) == 0) ? Color.Red : Color.Green;
            not_use_3.BackColor = (((LineNotUse << 5) & 0x80) == 0) ? Color.Red : Color.Green;
            not_use_4.BackColor = (((LineNotUse << 4) & 0x80) == 0) ? Color.Red : Color.Green;
            not_use_5.BackColor = (((LineNotUse << 3) & 0x80) == 0) ? Color.Red : Color.Green;
            not_use_6.BackColor = (((LineNotUse << 2) & 0x80) == 0) ? Color.Red : Color.Green;
            not_use_7.BackColor = (((LineNotUse << 1) & 0x80) == 0) ? Color.Red : Color.Green;
            not_use_8.BackColor = (((LineNotUse << 0) & 0x80) == 0) ? Color.Red : Color.Green;
            PublishMessage(topic_public[0], LineNotUse);
            Console.WriteLine(LineNotUse.ToString("X2"));

        }

        private void not_use_8_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[7] = !is_not_use_btn_press[7];
            if (is_not_use_btn_press[7] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0x7f);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x80);
            }
            this.Invoke(new EventHandler(HienThi));
        }

        private void not_use_6_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[5] = !is_not_use_btn_press[5];
            if (is_not_use_btn_press[5] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0xdf);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x20);
            }
            this.Invoke(new EventHandler(HienThi));
        }

        private void not_use_7_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[6] = !is_not_use_btn_press[6];
            if (is_not_use_btn_press[6] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0xbf);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x40);
            }
            this.Invoke(new EventHandler(HienThi));
        }

        private void not_use_5_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[4] = !is_not_use_btn_press[4];
            if (is_not_use_btn_press[4] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0xef);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x10);
            }
            this.Invoke(new EventHandler(HienThi));
        }

        private void not_use_3_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[2] = !is_not_use_btn_press[2];
            if (is_not_use_btn_press[2] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0xfb);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x04);
            }
            this.Invoke(new EventHandler(HienThi));
        }

        private void not_use_4_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[3] = !is_not_use_btn_press[3];
            if (is_not_use_btn_press[3] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0xf7);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x08);
            }
            this.Invoke(new EventHandler(HienThi));
        }

        private void not_use_2_Click(object sender, EventArgs e)
        {
            is_not_use_btn_press[1] = !is_not_use_btn_press[1];
            if (is_not_use_btn_press[1] == true)
            {
                LineNotUse = (byte)(LineNotUse & 0xfd);
            }
            else
            {
                LineNotUse = (byte)(LineNotUse | 0x02);
            }
            this.Invoke(new EventHandler(HienThi));
        }
    }
}
