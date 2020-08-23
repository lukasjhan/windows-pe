using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BasicApp.NET
{
	public partial class FormMain : Form
	{
		private static string _szOutBuf = "The Most Simple Windows GUI Program by YHD.";

		public FormMain()
		{
			InitializeComponent();
		}

		private void FormMain_Paint(object sender, PaintEventArgs e)
		{
			Graphics g = e.Graphics;

			Brush br = new SolidBrush(this.ForeColor);
			Rectangle rc = this.ClientRectangle;
			RectangleF dr = new RectangleF((float)rc.Left, (float)rc.Top,
											(float)rc.Width, (float)rc.Height);
			StringFormat df = new StringFormat();
			df.Alignment = StringAlignment.Center;
			df.LineAlignment = StringAlignment.Center;
			g.DrawString(_szOutBuf, this.Font, br, dr, df);
		}
	}
}
