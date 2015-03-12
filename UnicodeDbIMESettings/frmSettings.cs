using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.IO;
using System.Security;
using System.Globalization;
using System.Runtime.InteropServices;

namespace UnicodeDbIMESettings
{
    public partial class frmSettings : Form
    {

        KeyCapture activationSequence;
        string searchKey;
        string clipboardKey;

        public const string RegistrySubKey = "Software\\UnicodeDbIME";

        public frmSettings()
        {
            InitializeComponent();

            LoadSettingsFromRegistry();
            LoadLocales();
            LoadInstalledProfiles();
            UpdateUI();
        }

        private void LoadLocales()
        {
            CultureInfo[] cultures =
                CultureInfo.GetCultures(CultureTypes.AllCultures);

            foreach (CultureInfo ci in cultures) {
                cboLocale.Items.Add(ci.EnglishName);
            }
        }
        private void LoadInstalledProfiles()
        {
        }
        private void LoadSettingsFromRegistry()
        {
            /* Default settings first */
            activationSequence = new KeyCapture(
                KeyCapture.TfModModifers.CONTROL |
                KeyCapture.TfModModifers.SHIFT,
                Keys.U
                );
            searchKey = "'";
            clipboardKey = "v";

            /* Read activation sequence */
            RegistryKey rkHKCU = Microsoft.Win32.Registry.CurrentUser;
            RegistryKey rkUDBIME;

            try
            {

                rkUDBIME = rkHKCU.OpenSubKey(RegistrySubKey, false);
                if (rkUDBIME == null)
                    return;

                if (rkUDBIME.GetValueKind("ActivationSequenceModifiers") == RegistryValueKind.DWord)
                {
                    int aseqmod = (int)rkUDBIME.GetValue("ActivationSequenceModifiers");
                    activationSequence.currentModifiers = (KeyCapture.TfModModifers) (aseqmod);
                }

                if (rkUDBIME.GetValueKind("ActivationSequenceVKey") == RegistryValueKind.DWord)
                {
                    int aseqvk = (int)rkUDBIME.GetValue("ActivationSequenceVKey");
                    activationSequence.currentKey = (Keys)(aseqvk);
                }

                if (rkUDBIME.GetValueKind("SearchKey") == RegistryValueKind.String)
                {
                    string sk = (string)rkUDBIME.GetValue("SearchKey");
                    if (sk.Length > 0)
                        searchKey = sk;
                }

                if (rkUDBIME.GetValueKind("ClipboardKey") == RegistryValueKind.String)
                {
                    string ck = (string)rkUDBIME.GetValue("ClipboardKey");
                    if (ck.Length > 0)
                        clipboardKey = ck;
                }
            }
            catch (SecurityException sece)
            {
                return;
            }
            catch (ObjectDisposedException ode)
            {
                return;
            }
            catch (IOException ioe)
            {
                return;
            }
            catch (UnauthorizedAccessException uae)
            {
                return;
            }
        }

        private void UpdateUI()
        {
            txtActivationSequence.Text = activationSequence.ToString();
            txtSearchKey.Text = searchKey.Substring(0, 1);
            txtClipboard.Text = clipboardKey.Substring(0, 1);
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void btnCapture_Click(object sender, EventArgs e)
        {
            txtActivationSequence.Enabled = true;
            txtActivationSequence.Focus();
        }

        private void txtActivationSequence_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            // Accept all keys as input keys since we want to capture them
            e.IsInputKey = true;
        }

        private void txtActivationSequence_KeyDown(object sender, KeyEventArgs e)
        {
            // don't let the event bubble up
            e.Handled = true;
            e.SuppressKeyPress = true;

            /* Escape! */
            if (e.KeyCode == Keys.Escape && !(e.Control || e.Alt || e.Shift))
            {
                /* No change */
                txtActivationSequence.Text = activationSequence.ToString();
                txtActivationSequence.Enabled = false;
                return;
            }

            // Some other key -- get the keycode
            Keys key = e.KeyCode & Keys.KeyCode;

            activationSequence.capturingModifiers = 0;
            if (e.Shift && key != Keys.ShiftKey)
                activationSequence.capturingModifiers |= KeyCapture.TfModModifers.SHIFT;
            if (e.Control && key != Keys.ControlKey)
                activationSequence.capturingModifiers |= KeyCapture.TfModModifers.CONTROL;
            if (e.Alt && key != Keys.Menu)
                activationSequence.capturingModifiers |= KeyCapture.TfModModifers.ALT;
            
            activationSequence.capturingKey = key;

            // Some proper key pressed
            if (key != Keys.ControlKey && key != Keys.ShiftKey && key != Keys.Menu)
            {
                txtActivationSequence_KeyUp(null, e);
            }
            else // More keys may still be captured...
            {
                txtActivationSequence.Text = activationSequence.ToString(true);
            }
        }

        private void txtActivationSequence_Enter(object sender, EventArgs e)
        {
            txtActivationSequence.Text = "";
            btnCapture.Text = "Capturing... (ESC to cancel)";
            activationSequence.capturingKey = 0;
            activationSequence.capturingModifiers = 0;
        }

        class KeyCapture
        {
            /* From Microsoft documentation:
             * (Apparently the Windows Key is not supported by PreserveKey)
    TF_MOD_ALT
    0x0001
    Either of the ALT keys is pressed
    TF_MOD_CONTROL
    0x0002
    Either of the CTRL keys is pressed
    TF_MOD_SHIFT
    0x0004
    Either of the SHIFT keys is pressed
    TF_MOD_RALT
    0x0008
    The right ALT key is pressed
    TF_MOD_RCONTROL
    0x0010
    The right CTRL key is pressed
    TF_MOD_RSHIFT
    0x0020
    The right SHIFT key is pressed
    TF_MOD_LALT
    0x0040
    The left ALT key is pressed
    TF_MOD_LCONTROL
    0x0080
    The left CTRL key is pressed
    TF_MOD_LSHIFT
    0x0100
    The left SHIFT key is pressed
    TF_MOD_ON_KEYUP
    0x0200
    The event will be fired when the key is released. Without this flag, the event is fired when the key is pressed.
    TF_MOD_IGNORE_ALL_MODIFIER
    0x0400 */

            [System.FlagsAttribute]
            public enum TfModModifers
            {
                ALT = 0x0001,
                CONTROL = 0x0002,
                SHIFT = 0x0004,
                RALT = 0x0008,
                RCONTROL = 0x0010,
                RSHIFT = 0x0020,
                LALT = 0x0040,
                LCONTROL = 0x0080,
                LSHIFT = 0x0100,
                ON_KEYUP = 0x0200,
                IGNORE_ALL_MODIFIER = 0x0400,
            }

            public TfModModifers currentModifiers;
            public System.Windows.Forms.Keys currentKey;

            public TfModModifers capturingModifiers;
            public System.Windows.Forms.Keys capturingKey;

            public KeyCapture(TfModModifers modifiers, Keys key)
            {
                this.currentModifiers = modifiers;
                this.currentKey = key;
            }

            public override String ToString()
            {
                return ToString(false);
            }

            public String ToString(bool capturing)
            {
                TfModModifers mods = capturing ? capturingModifiers : currentModifiers;
                Keys key = capturing ? capturingKey : currentKey;
                StringBuilder sb = new StringBuilder();

                if (mods.HasFlag(TfModModifers.CONTROL))
                    sb.Append("Control+");
                if (mods.HasFlag(TfModModifers.ALT))
                    sb.Append("Alt+");
                if (mods.HasFlag(TfModModifers.SHIFT))
                    sb.Append("Shift+");

                sb.Append(key.ToString());

                return sb.ToString();
            }
        }

        private void txtActivationSequence_KeyUp(object sender, KeyEventArgs e)
        {
            activationSequence.currentKey = activationSequence.capturingKey;
            activationSequence.currentModifiers = activationSequence.capturingModifiers;

            // End capture
            txtActivationSequence.Enabled = false;
            txtActivationSequence.Text = activationSequence.ToString();
            btnCapture.Text = "C&apture...";
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            SaveSettings();
            this.Close();
        }

        private void SaveSettings()
        {
            /* Read activation sequence */
            RegistryKey rkHKCU = Microsoft.Win32.Registry.CurrentUser;
            RegistryKey rkUDBIME;

            try
            {

                rkUDBIME = rkHKCU.CreateSubKey(RegistrySubKey);
                if (rkUDBIME == null)
                    return;

                rkUDBIME.SetValue("ActivationSequenceModifiers",
                    activationSequence.currentModifiers,
                    RegistryValueKind.DWord);

                rkUDBIME.SetValue("ActivationSequenceVKey",
                    activationSequence.currentKey,
                    RegistryValueKind.DWord);

                rkUDBIME.SetValue("SearchKey",
                    searchKey.Substring(0, Math.Min(searchKey.Length, 1) ),
                    RegistryValueKind.String);
                
                rkUDBIME.SetValue("ClipboardKey",
                    clipboardKey.Substring(0, Math.Min(clipboardKey.Length, 1) ),
                    RegistryValueKind.String);
            }
            catch (SecurityException sece)
            {
                return;
            }
            catch (ObjectDisposedException ode)
            {
                return;
            }
            catch (IOException ioe)
            {
                return;
            }
            catch (UnauthorizedAccessException uae)
            {
                return;
            }
            /* Manipulate the registry here */
        }

        private void txtSearchKey_TextChanged(object sender, EventArgs e)
        {
            searchKey = txtSearchKey.Text;
        }

        private void txtClipboard_TextChanged(object sender, EventArgs e)
        {
            clipboardKey = txtClipboard.Text;
        }

        private void label2_Click_1(object sender, EventArgs e)
        {

        }
    }

    [ComImport, Guid(ProfileMgrCLSID)]
    class InputProcessorProfilesMgr
    {
    }
}
