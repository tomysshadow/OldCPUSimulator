using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace OldCPUSimulatorGUI {
    public partial class OldCPUSimulatorGUI : Form {
        private const string OLD_CPU_SIMULATOR_PATH = "OldCPUSimulator.exe";
        private const string CUSTOM_TARGET_RATE = "Custom Target Rate...";

        public OldCPUSimulatorGUI() {
            InitializeComponent();
        }

        private bool GetMaxMhz(out ulong maxMhz) {
            maxMhz = 0;

            // create the Get Max Rate Process to get the Max Rate
            ProcessStartInfo oldCPUSimulatorProcessStartInfo = new ProcessStartInfo(OLD_CPU_SIMULATOR_PATH, "--dev-get-max-mhz") {
                UseShellExecute = false,
                RedirectStandardError = false,
                RedirectStandardOutput = true,
                RedirectStandardInput = false,
                WindowStyle = ProcessWindowStyle.Hidden,
                CreateNoWindow = true,
                ErrorDialog = false
            };

            try {
                Process oldCPUSimulatorProcess = Process.Start(oldCPUSimulatorProcessStartInfo);

                if (!oldCPUSimulatorProcess.HasExited) {
                    oldCPUSimulatorProcess.WaitForExit();
                }

                string oldCPUSimulatorProcessStandardError = null;
                string oldCPUSimulatorProcessStandardOutput = null;

                if (oldCPUSimulatorProcessStartInfo.RedirectStandardError) {
                    oldCPUSimulatorProcessStandardError = oldCPUSimulatorProcess.StandardError.ReadToEnd();
                }

                if (oldCPUSimulatorProcessStartInfo.RedirectStandardOutput) {
                    oldCPUSimulatorProcessStandardOutput = oldCPUSimulatorProcess.StandardOutput.ReadToEnd();
                }

                if (oldCPUSimulatorProcess.ExitCode != 0 || !ulong.TryParse(oldCPUSimulatorProcessStandardOutput.Split('\n').Last(), out maxMhz)) {
                    MessageBox.Show(Properties.Resources.CPUSpeedNotDetermined, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }

                // set the Max Rate Value Label's Text to the Max Rate String
                maxMhzValueLabel.Text = maxMhz.ToString();
            } catch {
                MessageBox.Show(Properties.Resources.CPUSpeedNotDetermined, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }
            return true;
        }

        private bool GetMaxMhz() {
            return GetMaxMhz(out ulong maxMhz);
        }

        private readonly ulong[] targetMhzs = new ulong[] { 233, 350, 933 };

        private bool GetMhz(out ulong targetMhz, out ulong maxMhz) {
            targetMhz = 0;

            if (!GetMaxMhz(out maxMhz)) {
                Application.Exit();
                return false;
            }

            bool customTargetRate = false;

            // ensure the Target Rate Combo Box's Selected Item is less than the Max Rate
            if (targetMhzComboBox.DropDownStyle == ComboBoxStyle.DropDown) {
                if (!ulong.TryParse(targetMhzComboBox.Text, out targetMhz)) {
                    MessageBox.Show(Properties.Resources.TargetRateValidNumber, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }

                customTargetRate = true;
            } else if (targetMhzComboBox.DropDownStyle == ComboBoxStyle.DropDownList) {
                if (targetMhzComboBox.SelectedIndex < 0
                    || targetMhzComboBox.SelectedIndex >= targetMhzs.Length) {
                    targetMhzComboBox.SelectedIndex = 0;
                }

                targetMhz = targetMhzs[targetMhzComboBox.SelectedIndex];
            }

            if (targetMhz == 0) {
                MessageBox.Show(Properties.Resources.TargetRateNotZero, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            if (maxMhz <= targetMhz) {
                MessageBox.Show(String.Format(Properties.Resources.TargetRateMaxRate, maxMhz), Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            if (customTargetRate) {
                Properties.Settings.Default.TargetMhz = targetMhz;
                Properties.Settings.Default.Save();
            }
            return true;
        }

        void ShowRefreshRateMinimumMaximum() {
            if (refreshHzNumericUpDown.Value == refreshHzNumericUpDown.Maximum) {
                refreshHzMaximumNumericUpDown.Controls[0].Enabled = false;
                refreshHzMaximumGroupBox.Visible = true;
            } else {
                refreshHzMaximumGroupBox.Visible = false;
            }

            if (refreshHzNumericUpDown.Value == refreshHzNumericUpDown.Minimum) {
                refreshHzMinimumNumericUpDown.Controls[0].Enabled = false;
                refreshHzMinimumGroupBox.Visible = true;
            } else {
                refreshHzMinimumGroupBox.Visible = false;
            }
        }

        bool FloorRefreshRateFifteen(out ulong targetMhz, out ulong maxMhz) {
            if (!GetMhz(out targetMhz, out maxMhz)) {
                return false;
            }

            double suspend = (double)(maxMhz - targetMhz) / maxMhz;
            double resume = (double)targetMhz / maxMhz;

            refreshHzNumericUpDown.Increment = 1;
            refreshHzNumericUpDown.Minimum = 1;
            refreshHzNumericUpDown.Maximum = 1000;

            // if we're suspended 3 / 4
            // and resumed 1 / 4
            // we'll be suspended a minimum of 3 Ms
            // and resumed a minimum of 1 Ms
            // (3 / 4) / (1 / 4) = 3 Ms
            // 3 Ms + 1 Ms = 4 Ms, our minRefreshMs
            double minRefreshMs = (Math.Max(suspend, resume) / Math.Min(suspend, resume) * (double)refreshHzNumericUpDown.Minimum) + (double)refreshHzNumericUpDown.Minimum;
            double maxRefreshHz = (minRefreshMs > 0) ? ((double)refreshHzNumericUpDown.Maximum / Math.Ceiling(minRefreshMs)) : (double)refreshHzNumericUpDown.Maximum;

            refreshHzNumericUpDown.Value = MathUtils.Clamp((uint)Math.Min((double)Properties.Settings.Default.RefreshHz, maxRefreshHz), (uint)refreshHzNumericUpDown.Minimum, (uint)refreshHzNumericUpDown.Maximum);
            refreshHzNumericUpDown.Maximum = MathUtils.Clamp((uint)maxRefreshHz, (uint)refreshHzNumericUpDown.Minimum, (uint)refreshHzNumericUpDown.Maximum);

            // we do this after in case the Refresh Rate before was well above the maximum
            if (refreshRateFloorFifteenCheckBox.Checked) {
                maxRefreshHz = Math.Floor(maxRefreshHz / 15) * 15;

                refreshHzNumericUpDown.Minimum = 15;
                refreshHzNumericUpDown.Increment = 15;
                refreshHzNumericUpDown.Value = MathUtils.Clamp((uint)Math.Min((double)Math.Floor(refreshHzNumericUpDown.Value / 15) * 15, maxRefreshHz), (uint)refreshHzNumericUpDown.Minimum, (uint)refreshHzNumericUpDown.Maximum);
                refreshHzNumericUpDown.Maximum = MathUtils.Clamp((uint)maxRefreshHz, (uint)refreshHzNumericUpDown.Minimum, (uint)refreshHzNumericUpDown.Maximum);
            }

            ShowRefreshRateMinimumMaximum();
            return true;
        }

        void FloorRefreshRateFifteen() {
            FloorRefreshRateFifteen(out ulong targetMhz, out ulong maxMhz);
        }

        void SelectTargetMhz() {
            if (targetMhzComboBox.SelectedIndex == targetMhzComboBox.Items.IndexOf(CUSTOM_TARGET_RATE)) {
                targetMhzComboBox.DropDownStyle = ComboBoxStyle.DropDown;
                targetMhzComboBox.Text = Properties.Settings.Default.TargetMhz.ToString();
            } else {
                targetMhzComboBox.DropDownStyle = ComboBoxStyle.DropDownList;
            }
        }

        void ResetRefreshHz() {
            FloorRefreshRateFifteen();

            refreshHzNumericUpDown.Value = refreshHzNumericUpDown.Maximum;

            Properties.Settings.Default.RefreshHz = refreshHzNumericUpDown.Value;
            Properties.Settings.Default.Save();
        }

        // http://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
        public static string GetValidArgument(string argument, bool force = false) {
            if (argument == null) {
                argument = String.Empty;
            }

            if (!force && argument != String.Empty && argument.IndexOfAny(new char[] { ' ', '\t', '\n', '\v', '\"' }) == -1) {
                return argument;
            }

            int backslashes = 0;
            StringBuilder validArgument = new StringBuilder("\"");

            for (int i = 0; i < argument.Length; i++) {
                backslashes = 0;

                while (i != argument.Length && argument[i].ToString().Equals("\\", StringComparison.Ordinal)) {
                    backslashes++;
                    i++;
                }

                if (i != argument.Length) {
                    if (argument[i].ToString().Equals("\"", StringComparison.Ordinal)) {
                        validArgument.Append('\\', backslashes + backslashes + 1);
                    } else {
                        validArgument.Append('\\', backslashes);
                    }

                    validArgument.Append(argument[i]);
                }
            }

            validArgument.Append('\\', backslashes + backslashes);
            validArgument.Append("\"");
            return validArgument.ToString();
        }

        private void CreateOldCPUSimulatorProcess() {
            if (!FloorRefreshRateFifteen(out ulong targetMhz, out ulong maxMhz)) {
                return;
            }

            // create Arguments for the Old CPU Simulator Process Start Info
            StringBuilder oldCPUSimulatorProcessStartInfoArguments = new StringBuilder();
            oldCPUSimulatorProcessStartInfoArguments.Append(" -t ");
            oldCPUSimulatorProcessStartInfoArguments.Append(targetMhz);
            oldCPUSimulatorProcessStartInfoArguments.Append(" -r ");
            oldCPUSimulatorProcessStartInfoArguments.Append(refreshHzNumericUpDown.Value);

            if (setProcessPriorityHighCheckBox.Checked) {
                oldCPUSimulatorProcessStartInfoArguments.Append(" -ph");
            }

            if (setSyncedProcessAffinityOneCheckBox.Checked) {
                oldCPUSimulatorProcessStartInfoArguments.Append(" -a1");
            }

            if (syncedProcessMainThreadOnlyCheckBox.Checked) {
                oldCPUSimulatorProcessStartInfoArguments.Append(" -mt");
            }

            if (refreshRateFloorFifteenCheckBox.Checked) {
                oldCPUSimulatorProcessStartInfoArguments.Append(" -rf");
            }

            try {
                string fullPath = recentFilesListBox.GetItemText(recentFilesListBox.SelectedItem);

                if (String.IsNullOrEmpty(fullPath)) {
                    MessageBox.Show(Properties.Resources.SelectRecentFileFirst, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                fullPath = Path.GetFullPath(fullPath);

                // create the Old CPU Simulator Process Start Info
                oldCPUSimulatorProcessStartInfoArguments.Append(" -sw ");
                oldCPUSimulatorProcessStartInfoArguments.Append(GetValidArgument(fullPath, true));

                ProcessStartInfo oldCPUSimulatorProcessStartInfo = new ProcessStartInfo(OLD_CPU_SIMULATOR_PATH, oldCPUSimulatorProcessStartInfoArguments.ToString()) {
                    UseShellExecute = false,
                    RedirectStandardError = true,
                    RedirectStandardOutput = false,
                    RedirectStandardInput = false,
                    WindowStyle = ProcessWindowStyle.Hidden,
                    CreateNoWindow = true,
                    ErrorDialog = false,
                    WorkingDirectory = Path.GetDirectoryName(fullPath)
                };

                // create the Old CPU Simulator Process
                // hide... our laziness with not being async
                Hide();
                Process oldCPUSimulatorProcess = Process.Start(oldCPUSimulatorProcessStartInfo);

                if (!oldCPUSimulatorProcess.HasExited) {
                    oldCPUSimulatorProcess.WaitForExit();
                }

                Show();

                string oldCPUSimulatorProcessStandardError = null;
                string oldCPUSimulatorProcessStandardOutput = null;

                if (oldCPUSimulatorProcessStartInfo.RedirectStandardError) {
                    oldCPUSimulatorProcessStandardError = oldCPUSimulatorProcess.StandardError.ReadToEnd();
                }

                if (oldCPUSimulatorProcessStartInfo.RedirectStandardOutput) {
                    oldCPUSimulatorProcessStandardOutput = oldCPUSimulatorProcess.StandardOutput.ReadToEnd();
                }

                switch (oldCPUSimulatorProcess.ExitCode) {
                    case 0:
                    break;
                    case -1:
                    if (!String.IsNullOrEmpty(oldCPUSimulatorProcessStandardError)) {
                        string[] lastOldCPUSimulatorProcessStandardErrors = oldCPUSimulatorProcessStandardError.Split('\n');
                        string lastOldCPUSimulatorProcessStandardError = null;

                        if (lastOldCPUSimulatorProcessStandardErrors.Length > 1) {
                            lastOldCPUSimulatorProcessStandardError = lastOldCPUSimulatorProcessStandardErrors[lastOldCPUSimulatorProcessStandardErrors.Length - 2];
                        }

                        if (!String.IsNullOrEmpty(lastOldCPUSimulatorProcessStandardError)) {
                            MessageBox.Show(lastOldCPUSimulatorProcessStandardError, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                    }
                    break;
                    case -2:
                    MessageBox.Show(Properties.Resources.NoMultipleInstances, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                    case -3:
                    MessageBox.Show(Properties.Resources.CPUSpeedNotDetermined, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                    default:
                    MessageBox.Show(Properties.Resources.OldCPUNotSimulated, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                }
            } catch {
                Show();
                MessageBox.Show(Properties.Resources.OldCPUSimulatorProcessFailedCreate, Properties.Resources.OldCPUSimulator, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        void LoadSettings() {
            // fix bug with Recent button after Restoring Defaults
            recentFilesListBox.ResetText();

            recentFilesListBox.Items.Insert(0, Properties.Settings.Default.RecentFiles0);
            recentFilesListBox.Items.Insert(1, Properties.Settings.Default.RecentFiles1);
            recentFilesListBox.Items.Insert(2, Properties.Settings.Default.RecentFiles2);
            recentFilesListBox.Items.Insert(3, Properties.Settings.Default.RecentFiles3);
            recentFilesListBox.Items.Insert(4, Properties.Settings.Default.RecentFiles4);
            recentFilesListBox.Items.Insert(5, Properties.Settings.Default.RecentFiles5);
            recentFilesListBox.Items.Insert(6, Properties.Settings.Default.RecentFiles6);
            recentFilesListBox.Items.Insert(7, Properties.Settings.Default.RecentFiles7);
            recentFilesListBox.Items.Insert(8, Properties.Settings.Default.RecentFiles8);
            recentFilesListBox.Items.Insert(9, Properties.Settings.Default.RecentFiles9);

            quickReferenceLinkLabel.LinkVisited = Properties.Settings.Default.QuickReferenceLinkVisited;

            setProcessPriorityHighCheckBox.Checked = Properties.Settings.Default.SetProcessPriorityHigh;
            setSyncedProcessAffinityOneCheckBox.Checked = Properties.Settings.Default.SetSyncedProcessAffinityOne;
            syncedProcessMainThreadOnlyCheckBox.Checked = Properties.Settings.Default.SyncedProcessMainThreadOnly;
            refreshRateFloorFifteenCheckBox.Checked = Properties.Settings.Default.RefreshRateFloorFifteen;

            targetMhzComboBox.SelectedIndex = Properties.Settings.Default.TargetMhzSelectedIndex;

            SelectTargetMhz();
            FloorRefreshRateFifteen();
        }

        private void Open(object droppedFileName = null) {
            string fileName = droppedFileName as string;

            if (fileName == null) {
                if (openFileDialog.ShowDialog() == DialogResult.Cancel) {
                    return;
                }

                fileName = openFileDialog.FileName;
            } else {
                if (MessageBox.Show(Properties.Resources.RunDragAndDroppedFile, Properties.Resources.OldCPUSimulator, MessageBoxButtons.YesNo, MessageBoxIcon.None) == DialogResult.No) {
                    return;
                }
            }

            const int RECENT_FILES_END_INDEX = 9;

            bool itemRemoved = false;

            for (int i = 0; i < RECENT_FILES_END_INDEX; i++) {
                if (fileName == recentFilesListBox.Items[i].ToString()) {
                    recentFilesListBox.Items.RemoveAt(i);
                    itemRemoved = true;
                    break;
                }
            }

            if (!itemRemoved) {
                recentFilesListBox.Items.RemoveAt(RECENT_FILES_END_INDEX);
            }

            recentFilesListBox.Items.Insert(0, fileName);
            recentFilesListBox.SelectedIndex = 0;

            Properties.Settings.Default.RecentFiles0 = recentFilesListBox.Items[0].ToString();
            Properties.Settings.Default.RecentFiles1 = recentFilesListBox.Items[1].ToString();
            Properties.Settings.Default.RecentFiles2 = recentFilesListBox.Items[2].ToString();
            Properties.Settings.Default.RecentFiles3 = recentFilesListBox.Items[3].ToString();
            Properties.Settings.Default.RecentFiles4 = recentFilesListBox.Items[4].ToString();
            Properties.Settings.Default.RecentFiles5 = recentFilesListBox.Items[5].ToString();
            Properties.Settings.Default.RecentFiles6 = recentFilesListBox.Items[6].ToString();
            Properties.Settings.Default.RecentFiles7 = recentFilesListBox.Items[7].ToString();
            Properties.Settings.Default.RecentFiles8 = recentFilesListBox.Items[8].ToString();
            Properties.Settings.Default.RecentFiles9 = recentFilesListBox.Items[9].ToString();
            Properties.Settings.Default.Save();

            CreateOldCPUSimulatorProcess();
        }

        private void Recent() {
            CreateOldCPUSimulatorProcess();
        }

        private void WindowDragEnterOver(DragEventArgs e) {
            if (!e.Data.GetDataPresent(DataFormats.FileDrop)) {
                e.Effect = DragDropEffects.None;
                return;
            }

            e.Effect = DragDropEffects.Move;
        }

        private void WindowDragDrop(DragEventArgs e) {
            if (!e.Data.GetDataPresent(DataFormats.FileDrop)) {
                return;
            }

            if (!(e.Data.GetData(DataFormats.FileDrop) is string[] fileNames)) {
                return;
            }

            if (fileNames.Length < 1) {
                return;
            }
            
            // let the cursor effects disable before opening
            SynchronizationContext.Current.Post(Open, fileNames[0]);
        }

        private void OldCPUSimulatorGUI_Load(object sender, EventArgs e) {
            titleLabel.Text += " " + typeof(OldCPUSimulatorGUI).Assembly.GetName().Version;
            Text = titleLabel.Text;

            try {
                Directory.SetCurrentDirectory(Application.StartupPath);
            } catch {
                // Fail silently.
            }

            LoadSettings();
        }

        private void OldCPUSimulatorGUI_DragEnter(object sender, DragEventArgs e) {
            WindowDragEnterOver(e);
        }

        private void OldCPUSimulatorGUI_DragOver(object sender, DragEventArgs e) {
            WindowDragEnterOver(e);
        }

        private void OldCPUSimulatorGUI_DragDrop(object sender, DragEventArgs e) {
            WindowDragDrop(e);
        }

        private void openButton_Click(object sender, EventArgs e) {
            Open();
        }

        private void recentButton_Click(object sender, EventArgs e) {
            Recent();
        }

        private void quickReferenceLinkLabel_Click(object sender, EventArgs e) {
            Process.Start("http://intel.com/pressroom/kits/quickrefyr.htm");

            quickReferenceLinkLabel.LinkVisited = true;

            Properties.Settings.Default.QuickReferenceLinkVisited = quickReferenceLinkLabel.LinkVisited;
            Properties.Settings.Default.Save();
        }

        private void targetMhzComboBox_SelectionChangeCommitted(object sender, EventArgs e) {
            Properties.Settings.Default.TargetMhzSelectedIndex = targetMhzComboBox.SelectedIndex;
            Properties.Settings.Default.Save();

            SelectTargetMhz();
            ResetRefreshHz();
        }

        private void targetMhzComboBox_TextUpdate(object sender, EventArgs e) {
            ResetRefreshHz();
        }

        private void refreshHzNumericUpDown_ValueChanged(object sender, EventArgs e) {
            ShowRefreshRateMinimumMaximum();

            Properties.Settings.Default.RefreshHz = refreshHzNumericUpDown.Value;
            Properties.Settings.Default.Save();
        }

        private void setProcessPriorityHighCheckBox_Click(object sender, EventArgs e) {
            Properties.Settings.Default.SetProcessPriorityHigh = setProcessPriorityHighCheckBox.Checked;
            Properties.Settings.Default.Save();
        }

        private void setSyncedProcessAffinityOneCheckBox_Click(object sender, EventArgs e) {
            Properties.Settings.Default.SetSyncedProcessAffinityOne = setSyncedProcessAffinityOneCheckBox.Checked;
            Properties.Settings.Default.Save();
        }

        private void syncedProcessMainThreadOnlyCheckBox_Click(object sender, EventArgs e) {
            Properties.Settings.Default.SyncedProcessMainThreadOnly = syncedProcessMainThreadOnlyCheckBox.Checked;
            Properties.Settings.Default.Save();
        }

        private void refreshRateFloorFifteenCheckBox_Click(object sender, EventArgs e) {
            FloorRefreshRateFifteen();

            Properties.Settings.Default.RefreshHz = refreshHzNumericUpDown.Value;
            Properties.Settings.Default.RefreshRateFloorFifteen = refreshRateFloorFifteenCheckBox.Checked;
            Properties.Settings.Default.Save();
        }

        private void restoreDefaultsButton_Click(object sender, EventArgs e) {
            if (MessageBox.Show(Properties.Resources.AreYouSureRestoreDefaults, Properties.Resources.OldCPUSimulator, MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No) {
                return;
            }

            Properties.Settings.Default.Reset();

            LoadSettings();
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData) {
            if (keyData == (Keys.Control | Keys.O)) {
                Open();
                return true;
            }

            if (keyData == (Keys.Control | Keys.R)) {
                Recent();
                return true;
            }
            return base.ProcessCmdKey(ref msg, keyData);
        }
    }
}
