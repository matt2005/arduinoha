/*
 * Gemaakt met SharpDevelop.
 * Gebruiker: Sander
 * Datum: 16-7-2009
 * Tijd: 19:13
 * 
 * Dit sjabloon wijzigen: Extra | Opties |Coderen | Standaard kop bewerken.
 */
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace RFPlot
{
	/// <summary>
	/// Description of MainForm.
	/// </summary>
	public partial class MainForm : Form
	{
		public MainForm()
		{
			//
			// The InitializeComponent() call is required for Windows Forms designer support.
			//
			InitializeComponent();
			
			//
			// TODO: Add constructor code after the InitializeComponent() call.
			//
		}
		
		void Button1Click(object sender, EventArgs e)
		{
			Bitmap bitmap = new Bitmap(4096,300);
			Graphics graphics = Graphics.FromImage(bitmap);
			graphics.Clear(Color.Black);
			string[] pulsedurations = textBox1.Text.Split(new char[]{' '} , StringSplitOptions.RemoveEmptyEntries);
			
			Pen zeropen = new Pen(Color.Green , 2);
			graphics.DrawLine(zeropen , 0 , 200 , bitmap.Width , 200);
			int x = 10;
			bool curstatehigh = true;
			Pen pen = new Pen(Color.White , 2);
			
			Pen pulsepen = new Pen(Color.Red , 2);
			foreach (string pulseduration in pulsedurations)
			{
				string cleanedduration = pulseduration.Replace("[","").Replace("]","");
				
				int duration ;
				if (int.TryParse(cleanedduration, out duration))
				{
					if (curstatehigh)
					{
						graphics.DrawLine( pulsepen , x , 0 , x , 300) ;						
					}
					graphics.DrawLine(pen , x , curstatehigh?200:100 , x , curstatehigh?100:200);
					
					int dx = duration / 2 ;
					graphics.DrawLine(pen , x , curstatehigh?100:200 , x + dx , curstatehigh?100:200 );
					x += dx;
					graphics.DrawLine(pen , x , curstatehigh?100:200 , x , curstatehigh?200:100);
					curstatehigh = !curstatehigh;
				}
			}
			graphics.Flush();
			
			bitmap.Save(Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + "\\" + DateTime.Now.ToString().Replace(":" , "_").Replace(",","_") + ".png");
		}
	}
}
