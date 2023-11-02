namespace OldCPUSimulatorGUI
{
	partial class OldCPUSimulatorGUI
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(OldCPUSimulatorGUI));
            this.recentFilesListBox = new System.Windows.Forms.ListBox();
            this.titleLabel = new System.Windows.Forms.Label();
            this.subtitleLabel = new System.Windows.Forms.Label();
            this.recentFilesGroupBox = new System.Windows.Forms.GroupBox();
            this.recentLabel = new System.Windows.Forms.Label();
            this.runRecentButton = new System.Windows.Forms.Button();
            this.optionsGroupBox = new System.Windows.Forms.GroupBox();
            this.refreshRateFloorFifteenCheckBox = new System.Windows.Forms.CheckBox();
            this.setSyncedProcessAffinityOneCheckBox = new System.Windows.Forms.CheckBox();
            this.setProcessPriorityHighCheckBox = new System.Windows.Forms.CheckBox();
            this.refreshHzGroupBox = new System.Windows.Forms.GroupBox();
            this.refreshHzMaximumGroupBox = new System.Windows.Forms.GroupBox();
            this.refreshHzMaximumNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.refreshHzMinimumGroupBox = new System.Windows.Forms.GroupBox();
            this.refreshHzMinimumNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.refreshHzNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.refreshHzLabel = new System.Windows.Forms.Label();
            this.syncedProcessMainThreadOnlyCheckBox = new System.Windows.Forms.CheckBox();
            this.targetMhzComboBox = new System.Windows.Forms.ComboBox();
            this.openButton = new System.Windows.Forms.Button();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.maxMhzLabel = new System.Windows.Forms.Label();
            this.targetMhzLabel = new System.Windows.Forms.Label();
            this.maxMhzValueLabel = new System.Windows.Forms.Label();
            this.helpToolTip = new System.Windows.Forms.ToolTip(this.components);
            this.maxMhzEllipsisLabel = new System.Windows.Forms.Label();
            this.quickReferenceLinkLabel = new System.Windows.Forms.LinkLabel();
            this.restoreDefaultsButton = new System.Windows.Forms.Button();
            this.openLabel = new System.Windows.Forms.Label();
            this.recentFilesGroupBox.SuspendLayout();
            this.optionsGroupBox.SuspendLayout();
            this.refreshHzGroupBox.SuspendLayout();
            this.refreshHzMaximumGroupBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.refreshHzMaximumNumericUpDown)).BeginInit();
            this.refreshHzMinimumGroupBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.refreshHzMinimumNumericUpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.refreshHzNumericUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // recentFilesListBox
            // 
            this.recentFilesListBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.recentFilesListBox.FormattingEnabled = true;
            this.recentFilesListBox.HorizontalScrollbar = true;
            this.recentFilesListBox.Location = new System.Drawing.Point(6, 19);
            this.recentFilesListBox.Name = "recentFilesListBox";
            this.recentFilesListBox.Size = new System.Drawing.Size(248, 82);
            this.recentFilesListBox.TabIndex = 0;
            this.recentFilesListBox.SelectedIndexChanged += new System.EventHandler(this.recentFilesListBox_SelectedIndexChanged);
            // 
            // titleLabel
            // 
            this.titleLabel.AutoSize = true;
            this.titleLabel.Font = new System.Drawing.Font("Trebuchet MS", 14F, System.Drawing.FontStyle.Bold);
            this.titleLabel.Location = new System.Drawing.Point(12, 9);
            this.titleLabel.Name = "titleLabel";
            this.titleLabel.Size = new System.Drawing.Size(174, 24);
            this.titleLabel.TabIndex = 0;
            this.titleLabel.Text = "Old CPU Simulator";
            // 
            // subtitleLabel
            // 
            this.subtitleLabel.AutoSize = true;
            this.subtitleLabel.Font = new System.Drawing.Font("Georgia", 11F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Italic))));
            this.subtitleLabel.ForeColor = System.Drawing.Color.DimGray;
            this.subtitleLabel.Location = new System.Drawing.Point(12, 33);
            this.subtitleLabel.Name = "subtitleLabel";
            this.subtitleLabel.Size = new System.Drawing.Size(152, 18);
            this.subtitleLabel.TabIndex = 1;
            this.subtitleLabel.Text = "By Anthony Kleine";
            // 
            // recentFilesGroupBox
            // 
            this.recentFilesGroupBox.Controls.Add(this.recentFilesListBox);
            this.recentFilesGroupBox.Controls.Add(this.recentLabel);
            this.recentFilesGroupBox.Controls.Add(this.runRecentButton);
            this.recentFilesGroupBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.recentFilesGroupBox.Location = new System.Drawing.Point(12, 83);
            this.recentFilesGroupBox.Name = "recentFilesGroupBox";
            this.recentFilesGroupBox.Size = new System.Drawing.Size(260, 136);
            this.recentFilesGroupBox.TabIndex = 6;
            this.recentFilesGroupBox.TabStop = false;
            this.recentFilesGroupBox.Text = "Recent Files";
            // 
            // recentLabel
            // 
            this.recentLabel.AutoSize = true;
            this.recentLabel.Location = new System.Drawing.Point(106, 112);
            this.recentLabel.Name = "recentLabel";
            this.recentLabel.Size = new System.Drawing.Size(148, 13);
            this.recentLabel.TabIndex = 5;
            this.recentLabel.Text = "Run the selected Recent File.";
            // 
            // runRecentButton
            // 
            this.runRecentButton.Enabled = false;
            this.runRecentButton.Location = new System.Drawing.Point(6, 107);
            this.runRecentButton.Name = "runRecentButton";
            this.runRecentButton.Size = new System.Drawing.Size(94, 23);
            this.runRecentButton.TabIndex = 4;
            this.runRecentButton.Text = "Run Recent";
            this.runRecentButton.UseVisualStyleBackColor = true;
            this.runRecentButton.Click += new System.EventHandler(this.runRecentButton_Click);
            // 
            // optionsGroupBox
            // 
            this.optionsGroupBox.Controls.Add(this.refreshRateFloorFifteenCheckBox);
            this.optionsGroupBox.Controls.Add(this.setSyncedProcessAffinityOneCheckBox);
            this.optionsGroupBox.Controls.Add(this.setProcessPriorityHighCheckBox);
            this.optionsGroupBox.Controls.Add(this.refreshHzGroupBox);
            this.optionsGroupBox.Controls.Add(this.refreshHzLabel);
            this.optionsGroupBox.Controls.Add(this.syncedProcessMainThreadOnlyCheckBox);
            this.optionsGroupBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.optionsGroupBox.Location = new System.Drawing.Point(12, 278);
            this.optionsGroupBox.Name = "optionsGroupBox";
            this.optionsGroupBox.Size = new System.Drawing.Size(260, 125);
            this.optionsGroupBox.TabIndex = 13;
            this.optionsGroupBox.TabStop = false;
            this.optionsGroupBox.Text = "Options";
            // 
            // refreshRateFloorFifteenCheckBox
            // 
            this.refreshRateFloorFifteenCheckBox.AutoSize = true;
            this.refreshRateFloorFifteenCheckBox.Checked = true;
            this.refreshRateFloorFifteenCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.refreshRateFloorFifteenCheckBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.refreshRateFloorFifteenCheckBox.Location = new System.Drawing.Point(129, 76);
            this.refreshRateFloorFifteenCheckBox.Name = "refreshRateFloorFifteenCheckBox";
            this.refreshRateFloorFifteenCheckBox.Size = new System.Drawing.Size(106, 43);
            this.refreshRateFloorFifteenCheckBox.TabIndex = 5;
            this.refreshRateFloorFifteenCheckBox.Text = "Round Down\r\nRefresh Rate\r\nto Nearest 15 Hz";
            this.helpToolTip.SetToolTip(this.refreshRateFloorFifteenCheckBox, "Rounds Refresh Rate to the nearest multiple of 15 if applicable.");
            this.refreshRateFloorFifteenCheckBox.UseVisualStyleBackColor = true;
            this.refreshRateFloorFifteenCheckBox.Click += new System.EventHandler(this.refreshRateFloorFifteenCheckBox_Click);
            // 
            // setSyncedProcessAffinityOneCheckBox
            // 
            this.setSyncedProcessAffinityOneCheckBox.AutoSize = true;
            this.setSyncedProcessAffinityOneCheckBox.Checked = true;
            this.setSyncedProcessAffinityOneCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.setSyncedProcessAffinityOneCheckBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.setSyncedProcessAffinityOneCheckBox.Location = new System.Drawing.Point(129, 40);
            this.setSyncedProcessAffinityOneCheckBox.Name = "setSyncedProcessAffinityOneCheckBox";
            this.setSyncedProcessAffinityOneCheckBox.Size = new System.Drawing.Size(122, 30);
            this.setSyncedProcessAffinityOneCheckBox.TabIndex = 3;
            this.setSyncedProcessAffinityOneCheckBox.Text = "Set Synced Process\r\nAffinity to One";
            this.helpToolTip.SetToolTip(this.setSyncedProcessAffinityOneCheckBox, "Set the process affinity of the synced process\r\nto one, which may make the speed " +
        "more consistent\r\nand prevent crashes.\r\nMay not work with newer games.");
            this.setSyncedProcessAffinityOneCheckBox.UseVisualStyleBackColor = true;
            this.setSyncedProcessAffinityOneCheckBox.Click += new System.EventHandler(this.setSyncedProcessAffinityOneCheckBox_Click);
            // 
            // setProcessPriorityHighCheckBox
            // 
            this.setProcessPriorityHighCheckBox.AutoSize = true;
            this.setProcessPriorityHighCheckBox.Checked = true;
            this.setProcessPriorityHighCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.setProcessPriorityHighCheckBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.setProcessPriorityHighCheckBox.Location = new System.Drawing.Point(6, 40);
            this.setProcessPriorityHighCheckBox.Name = "setProcessPriorityHighCheckBox";
            this.setProcessPriorityHighCheckBox.Size = new System.Drawing.Size(117, 30);
            this.setProcessPriorityHighCheckBox.TabIndex = 2;
            this.setProcessPriorityHighCheckBox.Text = "Set Process Priority\r\nto High";
            this.helpToolTip.SetToolTip(this.setProcessPriorityHighCheckBox, "Set the process priority of Old CPU Simulator to High,\r\nin order to potentially i" +
        "mprove the accuracy of the simulation.");
            this.setProcessPriorityHighCheckBox.UseVisualStyleBackColor = true;
            this.setProcessPriorityHighCheckBox.Click += new System.EventHandler(this.setProcessPriorityHighCheckBox_Click);
            // 
            // refreshHzGroupBox
            // 
            this.refreshHzGroupBox.Controls.Add(this.refreshHzMaximumGroupBox);
            this.refreshHzGroupBox.Controls.Add(this.refreshHzMinimumGroupBox);
            this.refreshHzGroupBox.Controls.Add(this.refreshHzNumericUpDown);
            this.refreshHzGroupBox.Location = new System.Drawing.Point(105, 14);
            this.refreshHzGroupBox.Margin = new System.Windows.Forms.Padding(0);
            this.refreshHzGroupBox.Name = "refreshHzGroupBox";
            this.refreshHzGroupBox.Padding = new System.Windows.Forms.Padding(0);
            this.refreshHzGroupBox.Size = new System.Drawing.Size(149, 20);
            this.refreshHzGroupBox.TabIndex = 1;
            this.refreshHzGroupBox.TabStop = false;
            // 
            // refreshHzMaximumGroupBox
            // 
            this.refreshHzMaximumGroupBox.Controls.Add(this.refreshHzMaximumNumericUpDown);
            this.refreshHzMaximumGroupBox.Location = new System.Drawing.Point(133, 0);
            this.refreshHzMaximumGroupBox.Margin = new System.Windows.Forms.Padding(0);
            this.refreshHzMaximumGroupBox.Name = "refreshHzMaximumGroupBox";
            this.refreshHzMaximumGroupBox.Padding = new System.Windows.Forms.Padding(0);
            this.refreshHzMaximumGroupBox.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.refreshHzMaximumGroupBox.Size = new System.Drawing.Size(16, 10);
            this.refreshHzMaximumGroupBox.TabIndex = 1;
            this.refreshHzMaximumGroupBox.TabStop = false;
            this.refreshHzMaximumGroupBox.Visible = false;
            // 
            // refreshHzMaximumNumericUpDown
            // 
            this.refreshHzMaximumNumericUpDown.Enabled = false;
            this.refreshHzMaximumNumericUpDown.Location = new System.Drawing.Point(-133, 0);
            this.refreshHzMaximumNumericUpDown.Name = "refreshHzMaximumNumericUpDown";
            this.refreshHzMaximumNumericUpDown.Size = new System.Drawing.Size(149, 20);
            this.refreshHzMaximumNumericUpDown.TabIndex = 0;
            this.helpToolTip.SetToolTip(this.refreshHzMaximumNumericUpDown, resources.GetString("refreshHzMaximumNumericUpDown.ToolTip"));
            // 
            // refreshHzMinimumGroupBox
            // 
            this.refreshHzMinimumGroupBox.Controls.Add(this.refreshHzMinimumNumericUpDown);
            this.refreshHzMinimumGroupBox.Location = new System.Drawing.Point(133, 10);
            this.refreshHzMinimumGroupBox.Margin = new System.Windows.Forms.Padding(0);
            this.refreshHzMinimumGroupBox.Name = "refreshHzMinimumGroupBox";
            this.refreshHzMinimumGroupBox.Padding = new System.Windows.Forms.Padding(0);
            this.refreshHzMinimumGroupBox.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.refreshHzMinimumGroupBox.Size = new System.Drawing.Size(16, 10);
            this.refreshHzMinimumGroupBox.TabIndex = 2;
            this.refreshHzMinimumGroupBox.TabStop = false;
            this.refreshHzMinimumGroupBox.Visible = false;
            // 
            // refreshHzMinimumNumericUpDown
            // 
            this.refreshHzMinimumNumericUpDown.Enabled = false;
            this.refreshHzMinimumNumericUpDown.Location = new System.Drawing.Point(-133, -10);
            this.refreshHzMinimumNumericUpDown.Name = "refreshHzMinimumNumericUpDown";
            this.refreshHzMinimumNumericUpDown.Size = new System.Drawing.Size(149, 20);
            this.refreshHzMinimumNumericUpDown.TabIndex = 0;
            this.helpToolTip.SetToolTip(this.refreshHzMinimumNumericUpDown, resources.GetString("refreshHzMinimumNumericUpDown.ToolTip"));
            // 
            // refreshHzNumericUpDown
            // 
            this.refreshHzNumericUpDown.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.refreshHzNumericUpDown.Location = new System.Drawing.Point(0, 0);
            this.refreshHzNumericUpDown.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.refreshHzNumericUpDown.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.refreshHzNumericUpDown.Name = "refreshHzNumericUpDown";
            this.refreshHzNumericUpDown.Size = new System.Drawing.Size(149, 20);
            this.refreshHzNumericUpDown.TabIndex = 0;
            this.helpToolTip.SetToolTip(this.refreshHzNumericUpDown, resources.GetString("refreshHzNumericUpDown.ToolTip"));
            this.refreshHzNumericUpDown.Value = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.refreshHzNumericUpDown.ValueChanged += new System.EventHandler(this.refreshHzNumericUpDown_ValueChanged);
            // 
            // refreshHzLabel
            // 
            this.refreshHzLabel.AutoSize = true;
            this.refreshHzLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.refreshHzLabel.Location = new System.Drawing.Point(6, 16);
            this.refreshHzLabel.Name = "refreshHzLabel";
            this.refreshHzLabel.Size = new System.Drawing.Size(92, 13);
            this.refreshHzLabel.TabIndex = 0;
            this.refreshHzLabel.Text = "Refresh Rate (Hz)";
            this.helpToolTip.SetToolTip(this.refreshHzLabel, resources.GetString("refreshHzLabel.ToolTip"));
            // 
            // syncedProcessMainThreadOnlyCheckBox
            // 
            this.syncedProcessMainThreadOnlyCheckBox.AutoSize = true;
            this.syncedProcessMainThreadOnlyCheckBox.Checked = true;
            this.syncedProcessMainThreadOnlyCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.syncedProcessMainThreadOnlyCheckBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.syncedProcessMainThreadOnlyCheckBox.Location = new System.Drawing.Point(6, 82);
            this.syncedProcessMainThreadOnlyCheckBox.Name = "syncedProcessMainThreadOnlyCheckBox";
            this.syncedProcessMainThreadOnlyCheckBox.Size = new System.Drawing.Size(110, 30);
            this.syncedProcessMainThreadOnlyCheckBox.TabIndex = 4;
            this.syncedProcessMainThreadOnlyCheckBox.Text = "Synced Process\r\nMain Thread Only";
            this.helpToolTip.SetToolTip(this.syncedProcessMainThreadOnlyCheckBox, "This is an optimization which improves the accuracy of the\r\nsimulation, but may n" +
        "ot work well with multithreaded software.");
            this.syncedProcessMainThreadOnlyCheckBox.UseVisualStyleBackColor = true;
            this.syncedProcessMainThreadOnlyCheckBox.Click += new System.EventHandler(this.syncedProcessMainThreadOnlyCheckBox_Click);
            // 
            // targetMhzComboBox
            // 
            this.targetMhzComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.targetMhzComboBox.FormattingEnabled = true;
            this.targetMhzComboBox.Items.AddRange(new object[] {
            "100 MHz",
            "133 MHz",
            "166 MHz",
            "200 MHz",
            "233 MHz",
            "266 MHz",
            "300 MHz",
            "333 MHz",
            "350 MHz",
            "400 MHz",
            "450 MHz",
            "Custom Target Rate..."});
            this.targetMhzComboBox.Location = new System.Drawing.Point(12, 238);
            this.targetMhzComboBox.Name = "targetMhzComboBox";
            this.targetMhzComboBox.Size = new System.Drawing.Size(260, 21);
            this.targetMhzComboBox.TabIndex = 9;
            this.helpToolTip.SetToolTip(this.targetMhzComboBox, "The Target Rate (in MHz, from 1 to your CPU\'s clock speed)\r\nto simulate.");
            this.targetMhzComboBox.SelectionChangeCommitted += new System.EventHandler(this.targetMhzComboBox_SelectionChangeCommitted);
            this.targetMhzComboBox.TextUpdate += new System.EventHandler(this.targetMhzComboBox_TextUpdate);
            // 
            // openButton
            // 
            this.openButton.Location = new System.Drawing.Point(12, 54);
            this.openButton.Name = "openButton";
            this.openButton.Size = new System.Drawing.Size(81, 23);
            this.openButton.TabIndex = 2;
            this.openButton.Text = "Open";
            this.openButton.UseVisualStyleBackColor = true;
            this.openButton.Click += new System.EventHandler(this.openButton_Click);
            // 
            // openFileDialog
            // 
            this.openFileDialog.Filter = "Executable Files (*.exe)|*.exe|All Files (*.*)|*.*";
            // 
            // maxMhzLabel
            // 
            this.maxMhzLabel.AutoSize = true;
            this.maxMhzLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.maxMhzLabel.Location = new System.Drawing.Point(12, 262);
            this.maxMhzLabel.Name = "maxMhzLabel";
            this.maxMhzLabel.Size = new System.Drawing.Size(84, 13);
            this.maxMhzLabel.TabIndex = 10;
            this.maxMhzLabel.Text = "Max Rate (MHz)";
            this.helpToolTip.SetToolTip(this.maxMhzLabel, "The Max Rate (in MHz, your CPU\'s clock speed.)");
            // 
            // targetMhzLabel
            // 
            this.targetMhzLabel.AutoSize = true;
            this.targetMhzLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.targetMhzLabel.Location = new System.Drawing.Point(12, 222);
            this.targetMhzLabel.Name = "targetMhzLabel";
            this.targetMhzLabel.Size = new System.Drawing.Size(95, 13);
            this.targetMhzLabel.TabIndex = 7;
            this.targetMhzLabel.Text = "Target Rate (MHz)";
            this.helpToolTip.SetToolTip(this.targetMhzLabel, "The Target Rate (in MHz, from 1 to your CPU\'s clock speed)\r\nto simulate.");
            // 
            // maxMhzValueLabel
            // 
            this.maxMhzValueLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.maxMhzValueLabel.Location = new System.Drawing.Point(229, 262);
            this.maxMhzValueLabel.Name = "maxMhzValueLabel";
            this.maxMhzValueLabel.Size = new System.Drawing.Size(43, 13);
            this.maxMhzValueLabel.TabIndex = 12;
            this.maxMhzValueLabel.Text = "999999";
            this.maxMhzValueLabel.TextAlign = System.Drawing.ContentAlignment.TopRight;
            this.helpToolTip.SetToolTip(this.maxMhzValueLabel, "The Max Rate (in MHz, your CPU\'s clock speed.)");
            // 
            // helpToolTip
            // 
            this.helpToolTip.AutomaticDelay = 0;
            this.helpToolTip.AutoPopDelay = 3600000;
            this.helpToolTip.InitialDelay = 0;
            this.helpToolTip.ReshowDelay = 0;
            this.helpToolTip.UseAnimation = false;
            this.helpToolTip.UseFading = false;
            // 
            // maxMhzEllipsisLabel
            // 
            this.maxMhzEllipsisLabel.AutoSize = true;
            this.maxMhzEllipsisLabel.ForeColor = System.Drawing.SystemColors.GrayText;
            this.maxMhzEllipsisLabel.Location = new System.Drawing.Point(102, 262);
            this.maxMhzEllipsisLabel.Name = "maxMhzEllipsisLabel";
            this.maxMhzEllipsisLabel.Size = new System.Drawing.Size(121, 13);
            this.maxMhzEllipsisLabel.TabIndex = 11;
            this.maxMhzEllipsisLabel.Text = "......................................";
            this.helpToolTip.SetToolTip(this.maxMhzEllipsisLabel, "The Max Rate (in MHz, your CPU\'s clock speed.)");
            // 
            // quickReferenceLinkLabel
            // 
            this.quickReferenceLinkLabel.AutoSize = true;
            this.quickReferenceLinkLabel.Location = new System.Drawing.Point(184, 222);
            this.quickReferenceLinkLabel.Name = "quickReferenceLinkLabel";
            this.quickReferenceLinkLabel.Size = new System.Drawing.Size(88, 13);
            this.quickReferenceLinkLabel.TabIndex = 8;
            this.quickReferenceLinkLabel.TabStop = true;
            this.quickReferenceLinkLabel.Text = "Quick Reference";
            this.helpToolTip.SetToolTip(this.quickReferenceLinkLabel, "Go to a quick reference of year to clock speed.");
            this.quickReferenceLinkLabel.Click += new System.EventHandler(this.quickReferenceLinkLabel_Click);
            // 
            // restoreDefaultsButton
            // 
            this.restoreDefaultsButton.Location = new System.Drawing.Point(178, 409);
            this.restoreDefaultsButton.Name = "restoreDefaultsButton";
            this.restoreDefaultsButton.Size = new System.Drawing.Size(94, 23);
            this.restoreDefaultsButton.TabIndex = 14;
            this.restoreDefaultsButton.Text = "Restore Defaults";
            this.restoreDefaultsButton.UseVisualStyleBackColor = true;
            this.restoreDefaultsButton.Click += new System.EventHandler(this.restoreDefaultsButton_Click);
            // 
            // openLabel
            // 
            this.openLabel.AutoSize = true;
            this.openLabel.Location = new System.Drawing.Point(99, 59);
            this.openLabel.Name = "openLabel";
            this.openLabel.Size = new System.Drawing.Size(173, 13);
            this.openLabel.TabIndex = 3;
            this.openLabel.Text = "Select an executable file and run it.";
            // 
            // OldCPUSimulatorGUI
            // 
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 444);
            this.Controls.Add(this.quickReferenceLinkLabel);
            this.Controls.Add(this.openLabel);
            this.Controls.Add(this.maxMhzEllipsisLabel);
            this.Controls.Add(this.restoreDefaultsButton);
            this.Controls.Add(this.maxMhzValueLabel);
            this.Controls.Add(this.targetMhzLabel);
            this.Controls.Add(this.maxMhzLabel);
            this.Controls.Add(this.openButton);
            this.Controls.Add(this.optionsGroupBox);
            this.Controls.Add(this.subtitleLabel);
            this.Controls.Add(this.titleLabel);
            this.Controls.Add(this.recentFilesGroupBox);
            this.Controls.Add(this.targetMhzComboBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.KeyPreview = true;
            this.MaximizeBox = false;
            this.Name = "OldCPUSimulatorGUI";
            this.Text = "Old CPU Simulator";
            this.Load += new System.EventHandler(this.OldCPUSimulatorGUI_Load);
            this.DragDrop += new System.Windows.Forms.DragEventHandler(this.OldCPUSimulatorGUI_DragDrop);
            this.DragEnter += new System.Windows.Forms.DragEventHandler(this.OldCPUSimulatorGUI_DragEnter);
            this.DragOver += new System.Windows.Forms.DragEventHandler(this.OldCPUSimulatorGUI_DragOver);
            this.recentFilesGroupBox.ResumeLayout(false);
            this.recentFilesGroupBox.PerformLayout();
            this.optionsGroupBox.ResumeLayout(false);
            this.optionsGroupBox.PerformLayout();
            this.refreshHzGroupBox.ResumeLayout(false);
            this.refreshHzMaximumGroupBox.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.refreshHzMaximumNumericUpDown)).EndInit();
            this.refreshHzMinimumGroupBox.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.refreshHzMinimumNumericUpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.refreshHzNumericUpDown)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ListBox recentFilesListBox;
		private System.Windows.Forms.Label titleLabel;
		private System.Windows.Forms.Label subtitleLabel;
		private System.Windows.Forms.GroupBox recentFilesGroupBox;
		private System.Windows.Forms.GroupBox optionsGroupBox;
		private System.Windows.Forms.CheckBox setProcessPriorityHighCheckBox;
		private System.Windows.Forms.Label refreshHzLabel;
		private System.Windows.Forms.ComboBox targetMhzComboBox;
		private System.Windows.Forms.CheckBox setSyncedProcessAffinityOneCheckBox;
		private System.Windows.Forms.CheckBox syncedProcessMainThreadOnlyCheckBox;
		private System.Windows.Forms.CheckBox refreshRateFloorFifteenCheckBox;
		private System.Windows.Forms.Button openButton;
		private System.Windows.Forms.Button runRecentButton;
		private System.Windows.Forms.NumericUpDown refreshHzNumericUpDown;
		private System.Windows.Forms.OpenFileDialog openFileDialog;
		private System.Windows.Forms.Label maxMhzLabel;
		private System.Windows.Forms.Label targetMhzLabel;
		private System.Windows.Forms.Label maxMhzValueLabel;
        private System.Windows.Forms.GroupBox refreshHzMaximumGroupBox;
        private System.Windows.Forms.NumericUpDown refreshHzMaximumNumericUpDown;
        private System.Windows.Forms.GroupBox refreshHzMinimumGroupBox;
        private System.Windows.Forms.NumericUpDown refreshHzMinimumNumericUpDown;
        private System.Windows.Forms.ToolTip helpToolTip;
        private System.Windows.Forms.GroupBox refreshHzGroupBox;
        private System.Windows.Forms.Button restoreDefaultsButton;
        private System.Windows.Forms.Label maxMhzEllipsisLabel;
        private System.Windows.Forms.Label openLabel;
        private System.Windows.Forms.Label recentLabel;
        private System.Windows.Forms.LinkLabel quickReferenceLinkLabel;
    }
}

