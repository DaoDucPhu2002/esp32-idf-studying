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
     
        byte Rssi;
        int temp, smoke,out1,out2,nguong_nhiet,nguong_khoi;
        byte Status;
        string[] topic_public = new string[1];
        byte[] data_Send = new byte[2];
        public Form1()
        {
            InitializeComponent();
            topic_public[0] = "DaoDucPhu/alarm_setting";    
            mqClient = new MqttClient("broker.emqx.io"); // MQTT Broker
            mqClient.Connect("");
            if (mqClient.IsConnected ==  true)
            {
                mqClient.MqttMsgPublishReceived += MqClient_MqttMsgPublishRec;
                string[] topics = new string[1];
                topics[0] = "DaoDucPhu/alarm_noti";
                byte[] msg = new byte[1];
                msg[0] = MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE;
                
                mqClient.Subscribe(topics, msg);
            }
        }

     

        private void MqClient_MqttMsgPublishRec(object sender, MqttMsgPublishEventArgs e)
        {
          string topic = e.Topic;
           
            byte[] byteArray = e.Message;
           // string hex_string = BitConverter.ToString(byteArray).Replace("-","");
            if (topic == "DaoDucPhu/alarm_noti")
            {
              

                temp = byteArray[0];
                smoke = byteArray[1];
                Status = byteArray[2];
                Rssi = byteArray[3];
                
                out1 = (byteArray[4]==(byte)'1')?1:0;
                out2 = (byteArray[5] == (byte)'1') ? 1 : 0;
                //nguong
                nguong_khoi = byteArray[6];
                nguong_nhiet = byteArray[7];
                this.Invoke(new EventHandler(show_Status));

            }
        }

        private void show_Status(object sender, EventArgs e)
        {
            textBox_temp.Text = temp.ToString();
            textBox_smoke.Text = smoke.ToString();
            button_rl1.BackColor = (out1==1)?Color.Green:Color.Red;
            button_rl2.BackColor = (out2==1)?Color.Green: Color.Red;
            label_RSSI.Text = Rssi.ToString();
            label_khoi.Text = nguong_khoi.ToString();
            label_to.Text = nguong_nhiet.ToString();
            button_trangthai.BackColor = (Status==1)?Color.Red:Color.Green;
        }

        private void button_setting_Click(object sender, EventArgs e)
        {
            
            if (mqClient.IsConnected == true)
            {
                data_Send[0] = Convert.ToByte(textBox_set_smoke.Text);
                data_Send[1] = Convert.ToByte(textBox_set_t.Text);
                PublishMessage(topic_public[0], data_Send);
            }
        }

        void PublishMessage(string topic, byte[] byteValue)
        {
            mqClient.Publish(topic, byteValue, MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, false);

        }

     
        private void label2_Click(object sender, EventArgs e)
        {

        }
    }
}
