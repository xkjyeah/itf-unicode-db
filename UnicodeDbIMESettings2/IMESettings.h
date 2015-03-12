#pragma once

#include "KeyCaptureClass.h"
#include <msctf.h>
#include <ObjBase.h>

namespace UnicodeDbIMESettings2 {

	using namespace System;
	using namespace System::IO;
	using namespace System::Security;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace Microsoft::Win32;
	using namespace System::Globalization;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class SettingsForm : public System::Windows::Forms::Form
	{
	public:
		SettingsForm(void)
		{
			InitializeComponent();

            LoadSettingsFromRegistry();
            LoadLocales();
            LoadInstalledProfiles();

			UpdateUI();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SettingsForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::GroupBox^  groupBox2;
	protected: 
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel1;
	private: System::Windows::Forms::TextBox^  txtClipboard;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::TextBox^  txtSearchKey;
	private: System::Windows::Forms::Label^  lblSearchCharacter;
	private: System::Windows::Forms::Button^  btnCapture;
	private: System::Windows::Forms::TextBox^  txtActivationSequence;
	private: System::Windows::Forms::Label^  label1;


	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::Button^  btnOK;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->txtClipboard = (gcnew System::Windows::Forms::TextBox());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->txtSearchKey = (gcnew System::Windows::Forms::TextBox());
			this->lblSearchCharacter = (gcnew System::Windows::Forms::Label());
			this->btnCapture = (gcnew System::Windows::Forms::Button());
			this->txtActivationSequence = (gcnew System::Windows::Forms::TextBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOK = (gcnew System::Windows::Forms::Button());
			this->groupBox2->SuspendLayout();
			this->tableLayoutPanel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->tableLayoutPanel1);
			this->groupBox2->Location = System::Drawing::Point(7, 10);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(523, 119);
			this->groupBox2->TabIndex = 23;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Activation Keys";
			// 
			// tableLayoutPanel1
			// 
			this->tableLayoutPanel1->ColumnCount = 3;
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				43.59673F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				56.40327F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute, 
				145)));
			this->tableLayoutPanel1->Controls->Add(this->txtClipboard, 1, 2);
			this->tableLayoutPanel1->Controls->Add(this->label3, 0, 2);
			this->tableLayoutPanel1->Controls->Add(this->txtSearchKey, 1, 1);
			this->tableLayoutPanel1->Controls->Add(this->lblSearchCharacter, 0, 1);
			this->tableLayoutPanel1->Controls->Add(this->btnCapture, 2, 0);
			this->tableLayoutPanel1->Controls->Add(this->txtActivationSequence, 1, 0);
			this->tableLayoutPanel1->Controls->Add(this->label1, 0, 0);
			this->tableLayoutPanel1->Location = System::Drawing::Point(6, 19);
			this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
			this->tableLayoutPanel1->RowCount = 3;
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 61.79775F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 38.20225F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tableLayoutPanel1->Size = System::Drawing::Size(510, 87);
			this->tableLayoutPanel1->TabIndex = 18;
			// 
			// txtClipboard
			// 
			this->txtClipboard->Location = System::Drawing::Point(162, 63);
			this->txtClipboard->MaxLength = 1;
			this->txtClipboard->Name = L"txtClipboard";
			this->txtClipboard->Size = System::Drawing::Size(146, 20);
			this->txtClipboard->TabIndex = 16;
			this->txtClipboard->TextChanged += gcnew System::EventHandler(this, &SettingsForm::txtClipboard_TextChanged);
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(3, 60);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(151, 26);
			this->label3->TabIndex = 15;
			this->label3->Text = L"Clip&board inspection activation character";
			// 
			// txtSearchKey
			// 
			this->txtSearchKey->Location = System::Drawing::Point(162, 40);
			this->txtSearchKey->MaxLength = 1;
			this->txtSearchKey->Name = L"txtSearchKey";
			this->txtSearchKey->Size = System::Drawing::Size(146, 20);
			this->txtSearchKey->TabIndex = 14;
			this->txtSearchKey->TextChanged += gcnew System::EventHandler(this, &SettingsForm::txtSearchKey_TextChanged);
			// 
			// lblSearchCharacter
			// 
			this->lblSearchCharacter->AutoSize = true;
			this->lblSearchCharacter->Location = System::Drawing::Point(3, 37);
			this->lblSearchCharacter->Name = L"lblSearchCharacter";
			this->lblSearchCharacter->Size = System::Drawing::Size(138, 13);
			this->lblSearchCharacter->TabIndex = 13;
			this->lblSearchCharacter->Text = L"&Search activation character";
			// 
			// btnCapture
			// 
			this->btnCapture->Location = System::Drawing::Point(367, 3);
			this->btnCapture->Name = L"btnCapture";
			this->btnCapture->Size = System::Drawing::Size(129, 31);
			this->btnCapture->TabIndex = 12;
			this->btnCapture->Text = L"C&apture...";
			this->btnCapture->UseVisualStyleBackColor = true;
			this->btnCapture->Click += gcnew System::EventHandler(this, &SettingsForm::btnCapture_Click);
			// 
			// txtActivationSequence
			// 
			this->txtActivationSequence->Enabled = false;
			this->txtActivationSequence->ImeMode = System::Windows::Forms::ImeMode::Off;
			this->txtActivationSequence->Location = System::Drawing::Point(162, 3);
			this->txtActivationSequence->Name = L"txtActivationSequence";
			this->txtActivationSequence->Size = System::Drawing::Size(146, 20);
			this->txtActivationSequence->TabIndex = 11;
			this->txtActivationSequence->TextChanged += gcnew System::EventHandler(this, &SettingsForm::txtActivationSequence_TextChanged);
			this->txtActivationSequence->Enter += gcnew System::EventHandler(this, &SettingsForm::txtActivationSequence_Enter);
			this->txtActivationSequence->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &SettingsForm::txtActivationSequence_KeyDown);
			this->txtActivationSequence->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &SettingsForm::txtActivationSequence_KeyUp);
			this->txtActivationSequence->PreviewKeyDown += gcnew System::Windows::Forms::PreviewKeyDownEventHandler(this, &SettingsForm::txtActivationSequence_PreviewKeyDown);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(3, 0);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(125, 26);
			this->label1->TabIndex = 10;
			this->label1->Text = L"Unicode input activation sequence";
			// 
			// btnCancel
			// 
			this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->btnCancel->Location = System::Drawing::Point(435, 147);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(95, 28);
			this->btnCancel->TabIndex = 21;
			this->btnCancel->Text = L"&Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &SettingsForm::btnCancel_Click);
			// 
			// btnOK
			// 
			this->btnOK->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->btnOK->Location = System::Drawing::Point(334, 147);
			this->btnOK->Name = L"btnOK";
			this->btnOK->Size = System::Drawing::Size(95, 28);
			this->btnOK->TabIndex = 20;
			this->btnOK->Text = L"&OK";
			this->btnOK->UseVisualStyleBackColor = true;
			this->btnOK->Click += gcnew System::EventHandler(this, &SettingsForm::btnOK_Click);
			// 
			// SettingsForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(536, 191);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->btnOK);
			this->Name = L"SettingsForm";
			this->Text = L"Unicode DB IME Settings";
			this->groupBox2->ResumeLayout(false);
			this->tableLayoutPanel1->ResumeLayout(false);
			this->tableLayoutPanel1->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void txtSearchKey_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            _searchKey = txtSearchKey->Text;
			 }
private: System::Void txtClipboard_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            _clipboardKey = txtClipboard->Text;
		 }
private: System::Void txtActivationSequence_TextChanged(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void btnOK_Click(System::Object^  sender, System::EventArgs^  e) {
            SaveSettings();
            this->Close();
		 }
private: System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e) {
            this->Close();
		 }
private: System::Void txtActivationSequence_PreviewKeyDown(System::Object^  sender, System::Windows::Forms::PreviewKeyDownEventArgs^  e) {
            // Accept all keys as input keys since we want to capture them
            e->IsInputKey = true;
		 }
		 private: void ExitCapture() {
            txtActivationSequence->Enabled = false;
            txtActivationSequence->Text = activationSequence.ToString();
            btnCapture->Text = "C&apture...";
		}
		 void EnterCapture() {
            txtActivationSequence->Enabled = true;
            txtActivationSequence->Focus();
            btnCapture->Text = "Capturing... (ESC to cancel)";
		}
private: System::Void txtActivationSequence_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
            // don't let the event bubble up
            e->Handled = true;
            e->SuppressKeyPress = true;

            /* Escape! */
            if (e->KeyCode == Keys::Escape && !(e->Control || e->Alt || e->Shift))
            {
                /* No change */
				ExitCapture();
                return;
            }

            // Some other key -- get the keycode
            Keys key = e->KeyCode & Keys::KeyCode;

            activationSequence.capturingModifiers = 0;
            if (e->Shift && key != Keys::ShiftKey)
                activationSequence.capturingModifiers |= TF_MOD_SHIFT;
            if (e->Control && key != Keys::ControlKey)
                activationSequence.capturingModifiers |= TF_MOD_CONTROL;
            if (e->Alt && key != Keys::Menu)
                activationSequence.capturingModifiers |= TF_MOD_ALT;
            
            activationSequence.capturingKey = key;

            // Some proper key pressed
            if (key != Keys::ControlKey && key != Keys::ShiftKey && key != Keys::Menu)
            {
                txtActivationSequence_KeyUp(nullptr, e);
            }
            else // More keys may still be captured...
            {
                txtActivationSequence->Text = activationSequence.ToString(true);
            }
		 }
private: System::Void txtActivationSequence_Enter(System::Object^  sender, System::EventArgs^  e) {
            txtActivationSequence->Text = "";
            activationSequence.capturingKey = (Keys)0;
            activationSequence.capturingModifiers = 0;
		 }
private: System::Void txtActivationSequence_KeyUp(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
			 
            activationSequence.currentKey = activationSequence.capturingKey;
            activationSequence.currentModifiers = activationSequence.capturingModifiers;

            // End capture
			ExitCapture();
		 }

private: void SaveSettings()
        {
            /* Read activation sequence */
            RegistryKey^ rkHKCU = Microsoft::Win32::Registry::CurrentUser;
            RegistryKey^ rkUDBIME;

            try
            {
                rkUDBIME = rkHKCU->CreateSubKey(RegistrySubKey);
                if (rkUDBIME == nullptr)
                    return;

                rkUDBIME->SetValue(L"ActivationSequenceModifiers",
                    activationSequence.currentModifiers,
                    RegistryValueKind::DWord);

                rkUDBIME->SetValue(L"ActivationSequenceVKey",
                    activationSequence.currentKey,
                    RegistryValueKind::DWord);

                rkUDBIME->SetValue(L"SearchKey",
                    _searchKey->Substring(0, Math::Min(_searchKey->Length, 1) ),
                    RegistryValueKind::String);
                
                rkUDBIME->SetValue(L"ClipboardKey",
                    _clipboardKey->Substring(0, Math::Min(_clipboardKey->Length, 1) ),
                    RegistryValueKind::String);
            }
            catch (SecurityException^ sece)
            {
                return;
            }
            catch (ObjectDisposedException^ ode)
            {
                return;
            }
            catch (IOException^ ioe)
            {
                return;
            }
            catch (UnauthorizedAccessException^ uae)
            {
                return;
            }
            /* Manipulate the registry here */
        }
	private:
		void LoadInstalledProfiles() {
			HRESULT hr;

			ITfInputProcessorProfiles *pInputProcessProfiles;

			hr = CoCreateInstance(  CLSID_TF_InputProcessorProfiles, 
				NULL, 
				CLSCTX_INPROC_SERVER,
				IID_ITfInputProcessorProfiles, 
				(LPVOID*)&pInputProcessProfiles);

			if (!SUCCEEDED(hr)) {
				/* TODO: idk... throw some exception or something */
				return;
			}

			/* TODO: then add all the stuff */
		}
		void LoadLocales() {
			/*
            array<CultureInfo^>^ cultures =
                CultureInfo::GetCultures(CultureTypes::AllCultures);

            for (int i=0; i<cultures->Length; i++) {
                cboLocale->Items->Add(cultures[i]->EnglishName);
            }*/
		}
		void LoadSettingsFromRegistry() {
            /* Default settings first */
            activationSequence.currentModifiers = TF_MOD_CONTROL | TF_MOD_SHIFT;
			activationSequence.currentKey = Keys::U;
            _searchKey = L"'";
            _clipboardKey = L"v";

            /* Read activation sequence */
            RegistryKey^ rkHKCU = Microsoft::Win32::Registry::CurrentUser;
            RegistryKey^ rkUDBIME;

            try
            {

                rkUDBIME = rkHKCU->OpenSubKey(RegistrySubKey, false);
                if (rkUDBIME == nullptr)
                    return;

                if (rkUDBIME->GetValueKind(L"ActivationSequenceModifiers") == RegistryValueKind::DWord)
                {
                    int aseqmod = (int)rkUDBIME->GetValue(L"ActivationSequenceModifiers");
                    activationSequence.currentModifiers = aseqmod;
                }

                if (rkUDBIME->GetValueKind(L"ActivationSequenceVKey") == RegistryValueKind::DWord)
                {
                    int aseqvk = (int)rkUDBIME->GetValue(L"ActivationSequenceVKey");
                    activationSequence.currentKey = (Keys)(aseqvk);
                }

                if (rkUDBIME->GetValueKind(L"SearchKey") == RegistryValueKind::String)
                {
                    String^ sk = (String^)rkUDBIME->GetValue(L"SearchKey");
                    if (sk->Length > 0)
                        _searchKey = sk;
                }

                if (rkUDBIME->GetValueKind("ClipboardKey") == RegistryValueKind::String)
                {
                    String^ ck = (String^)rkUDBIME->GetValue(L"ClipboardKey");
                    if (ck->Length > 0)
                        _clipboardKey = ck;
                }
            }
            catch (SecurityException^ sece)
            {
                return;
            }
            catch (ObjectDisposedException ^ode)
            {
                return;
            }
            catch (IOException ^ioe)
            {
                return;
            }
            catch (UnauthorizedAccessException ^uae)
            {
                return;
            }
        }
		
        void UpdateUI()
        {
            txtActivationSequence->Text = activationSequence.ToString();
            txtSearchKey->Text = _searchKey->Substring(0, 1);
            txtClipboard->Text = _clipboardKey->Substring(0, 1);
        }
		 
	private:
		System::String^ _searchKey, ^_clipboardKey;
		KeyCapture activationSequence;
        static String^ RegistrySubKey = L"Software\\UnicodeDbIME";
private: System::Void btnCapture_Click(System::Object^  sender, System::EventArgs^  e) {
			 EnterCapture();
		 }
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void listBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}

