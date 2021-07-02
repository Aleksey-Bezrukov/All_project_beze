//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <IWBaseComponent.hpp>
#include <IWBaseHTML40Component.hpp>
#include <IWBaseHTMLComponent.hpp>
#include <IWCompExtCtrls.hpp>
 #define NOMINMAX //иначе API windows определит макросы min и max, конфликтующие с std::max и std::min в vector
 #include <windows.h>

 #include <vector>
 #include <string>
//---------------------------------------------------------------------------
 using namespace std;

 struct TTY {

 	TTY();
 	virtual ~TTY();

 	bool IsOK() const;

 	void Connect(const String port, int baudrate);
 	void Disconnect();

 	virtual void Write(const vector<unsigned char>& data);
 	virtual void Read(vector<unsigned char>& data);

 	HANDLE m_Handle;

 };

 struct TTYException {
 };

class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TButton *Button1;
	TEdit *Edit1;
	TButton *Button2;
	TTimer *Timer1;
	TLabel *Label2;
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Close(TObject *Sender, TCloseAction &Action);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
