using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace OldCPUSimulatorGUI
{
	public static class MathUtils
	{
		public static uint Clamp(uint number, uint min, uint max)
		{
			return Math.Min(max, Math.Max(min, number));
		}
	}

	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new OldCPUSimulatorGUI());
		}
	}
}
