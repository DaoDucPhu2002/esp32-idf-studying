namespace MQTT_V2
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.textBox_temp = new System.Windows.Forms.TextBox();
            this.textBox_smoke = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label_trangthai = new System.Windows.Forms.Label();
            this.button_trangthai = new System.Windows.Forms.Button();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.textBox_set_t = new System.Windows.Forms.TextBox();
            this.textBox_set_smoke = new System.Windows.Forms.TextBox();
            this.button_setting = new System.Windows.Forms.Button();
            this.label_to = new System.Windows.Forms.Label();
            this.label_khoi = new System.Windows.Forms.Label();
            this.button_rl1 = new System.Windows.Forms.Button();
            this.label10 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.button_rl2 = new System.Windows.Forms.Button();
            this.label12 = new System.Windows.Forms.Label();
            this.label_RSSI = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Times New Roman", 27.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(291, 31);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(433, 53);
            this.label1.TabIndex = 0;
            this.label1.Text = "Hệ Thống Báo Cháy";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(64, 220);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(167, 38);
            this.label2.TabIndex = 1;
            this.label2.Text = "Nhiệt Độ: ";
            this.label2.Click += new System.EventHandler(this.label2_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(64, 296);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(237, 38);
            this.label3.TabIndex = 1;
            this.label3.Text = "Nồng Độ Khói:";
            this.label3.Click += new System.EventHandler(this.label2_Click);
            // 
            // textBox_temp
            // 
            this.textBox_temp.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox_temp.Location = new System.Drawing.Point(350, 212);
            this.textBox_temp.Name = "textBox_temp";
            this.textBox_temp.Size = new System.Drawing.Size(110, 46);
            this.textBox_temp.TabIndex = 2;
            this.textBox_temp.Text = "0";
            this.textBox_temp.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // textBox_smoke
            // 
            this.textBox_smoke.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox_smoke.Location = new System.Drawing.Point(350, 288);
            this.textBox_smoke.Name = "textBox_smoke";
            this.textBox_smoke.Size = new System.Drawing.Size(110, 46);
            this.textBox_smoke.TabIndex = 2;
            this.textBox_smoke.Text = "0";
            this.textBox_smoke.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(64, 142);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(181, 38);
            this.label4.TabIndex = 1;
            this.label4.Text = "Trạng thái:";
            this.label4.Click += new System.EventHandler(this.label2_Click);
            // 
            // label_trangthai
            // 
            this.label_trangthai.AutoSize = true;
            this.label_trangthai.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label_trangthai.Location = new System.Drawing.Point(314, 142);
            this.label_trangthai.Name = "label_trangthai";
            this.label_trangthai.Size = new System.Drawing.Size(212, 38);
            this.label_trangthai.TabIndex = 1;
            this.label_trangthai.Text = "Bình Thường";
            this.label_trangthai.Click += new System.EventHandler(this.label2_Click);
            // 
            // button_trangthai
            // 
            this.button_trangthai.Location = new System.Drawing.Point(815, 31);
            this.button_trangthai.Name = "button_trangthai";
            this.button_trangthai.Size = new System.Drawing.Size(100, 100);
            this.button_trangthai.TabIndex = 3;
            this.button_trangthai.UseVisualStyleBackColor = true;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Times New Roman", 27.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(543, 155);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(461, 53);
            this.label6.TabIndex = 0;
            this.label6.Text = "Cài Đặt Ngưỡng Báo:";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(519, 215);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(167, 38);
            this.label7.TabIndex = 1;
            this.label7.Text = "Nhiệt Độ: ";
            this.label7.Click += new System.EventHandler(this.label2_Click);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label8.Location = new System.Drawing.Point(519, 291);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(237, 38);
            this.label8.TabIndex = 1;
            this.label8.Text = "Nồng Độ Khói:";
            this.label8.Click += new System.EventHandler(this.label2_Click);
            // 
            // textBox_set_t
            // 
            this.textBox_set_t.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox_set_t.Location = new System.Drawing.Point(805, 207);
            this.textBox_set_t.Name = "textBox_set_t";
            this.textBox_set_t.Size = new System.Drawing.Size(110, 46);
            this.textBox_set_t.TabIndex = 2;
            this.textBox_set_t.Text = "0";
            this.textBox_set_t.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // textBox_set_smoke
            // 
            this.textBox_set_smoke.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBox_set_smoke.Location = new System.Drawing.Point(805, 283);
            this.textBox_set_smoke.Name = "textBox_set_smoke";
            this.textBox_set_smoke.Size = new System.Drawing.Size(110, 46);
            this.textBox_set_smoke.TabIndex = 2;
            this.textBox_set_smoke.Text = "0";
            this.textBox_set_smoke.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // button_setting
            // 
            this.button_setting.Font = new System.Drawing.Font("Times New Roman", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button_setting.Location = new System.Drawing.Point(805, 365);
            this.button_setting.Name = "button_setting";
            this.button_setting.Size = new System.Drawing.Size(110, 42);
            this.button_setting.TabIndex = 4;
            this.button_setting.Text = "Thiết lập";
            this.button_setting.UseVisualStyleBackColor = true;
            this.button_setting.Click += new System.EventHandler(this.button_setting_Click);
            // 
            // label_to
            // 
            this.label_to.AutoSize = true;
            this.label_to.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label_to.Location = new System.Drawing.Point(757, 212);
            this.label_to.Name = "label_to";
            this.label_to.Size = new System.Drawing.Size(51, 38);
            this.label_to.TabIndex = 1;
            this.label_to.Text = "00";
            this.label_to.Click += new System.EventHandler(this.label2_Click);
            // 
            // label_khoi
            // 
            this.label_khoi.AutoSize = true;
            this.label_khoi.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label_khoi.Location = new System.Drawing.Point(757, 288);
            this.label_khoi.Name = "label_khoi";
            this.label_khoi.Size = new System.Drawing.Size(51, 38);
            this.label_khoi.TabIndex = 1;
            this.label_khoi.Text = "00";
            this.label_khoi.Click += new System.EventHandler(this.label2_Click);
            // 
            // button_rl1
            // 
            this.button_rl1.Location = new System.Drawing.Point(230, 357);
            this.button_rl1.Name = "button_rl1";
            this.button_rl1.Size = new System.Drawing.Size(50, 50);
            this.button_rl1.TabIndex = 3;
            this.button_rl1.UseVisualStyleBackColor = true;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label10.Location = new System.Drawing.Point(64, 362);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(136, 38);
            this.label10.TabIndex = 1;
            this.label10.Text = "Relay 1:";
            this.label10.Click += new System.EventHandler(this.label2_Click);
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label11.Location = new System.Drawing.Point(336, 362);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(136, 38);
            this.label11.TabIndex = 1;
            this.label11.Text = "Relay 2:";
            this.label11.Click += new System.EventHandler(this.label2_Click);
            // 
            // button_rl2
            // 
            this.button_rl2.Location = new System.Drawing.Point(502, 357);
            this.button_rl2.Name = "button_rl2";
            this.button_rl2.Size = new System.Drawing.Size(50, 50);
            this.button_rl2.TabIndex = 3;
            this.button_rl2.UseVisualStyleBackColor = true;
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label12.Location = new System.Drawing.Point(569, 100);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(202, 38);
            this.label12.TabIndex = 1;
            this.label12.Text = "RSSI WIFI: ";
            this.label12.Click += new System.EventHandler(this.label2_Click);
            // 
            // label_RSSI
            // 
            this.label_RSSI.AutoSize = true;
            this.label_RSSI.Font = new System.Drawing.Font("Times New Roman", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label_RSSI.Location = new System.Drawing.Point(741, 100);
            this.label_RSSI.Name = "label_RSSI";
            this.label_RSSI.Size = new System.Drawing.Size(51, 38);
            this.label_RSSI.TabIndex = 1;
            this.label_RSSI.Text = "00";
            this.label_RSSI.Click += new System.EventHandler(this.label2_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(980, 450);
            this.Controls.Add(this.button_setting);
            this.Controls.Add(this.button_rl2);
            this.Controls.Add(this.button_rl1);
            this.Controls.Add(this.button_trangthai);
            this.Controls.Add(this.textBox_set_smoke);
            this.Controls.Add(this.textBox_smoke);
            this.Controls.Add(this.textBox_set_t);
            this.Controls.Add(this.textBox_temp);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label_RSSI);
            this.Controls.Add(this.label12);
            this.Controls.Add(this.label_trangthai);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label_khoi);
            this.Controls.Add(this.label_to);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBox_temp;
        private System.Windows.Forms.TextBox textBox_smoke;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label_trangthai;
        private System.Windows.Forms.Button button_trangthai;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox textBox_set_t;
        private System.Windows.Forms.TextBox textBox_set_smoke;
        private System.Windows.Forms.Button button_setting;
        private System.Windows.Forms.Label label_to;
        private System.Windows.Forms.Label label_khoi;
        private System.Windows.Forms.Button button_rl1;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Button button_rl2;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label_RSSI;
    }
}

