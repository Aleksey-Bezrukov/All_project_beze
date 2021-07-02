//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"

#include <iostream>
#include <assert.h>
#include <windows.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
  using namespace std;

 static int TIMEOUT = 1000;

const int READ_TIME = 100;
OVERLAPPED sync = {0};
int reuslt = 0;
unsigned long wait = 0, read = 0, state = 0;

 TTY::TTY() {
 	m_Handle = INVALID_HANDLE_VALUE;
 }

 TTY::~TTY() {
 	Disconnect();
 }

  bool TTY::IsOK() const {
      return m_Handle != INVALID_HANDLE_VALUE;
  }

 void TTY::Connect(const String port, int baudrate) {

 	Disconnect();

 	m_Handle =
 		CreateFile(
 		port.c_str(),
 		GENERIC_READ | GENERIC_WRITE,
 		0,
 		NULL,
 		OPEN_EXISTING,
 	      FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //Асинхронный режим
 		NULL);

 	if(m_Handle == INVALID_HANDLE_VALUE) {
 		throw TTYException();
 	}

 	SetCommMask(m_Handle, EV_RXCHAR);
 	SetupComm(m_Handle, 1500, 1500);

 	COMMTIMEOUTS CommTimeOuts;
 	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
 	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
 	CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUT;
 	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
 	CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;

 	if(!SetCommTimeouts(m_Handle, &CommTimeOuts)) {
 		CloseHandle(m_Handle);
                m_Handle = INVALID_HANDLE_VALUE;
 		throw TTYException();
 	}

 	DCB ComDCM;

 	memset(&ComDCM,0,sizeof(ComDCM));
 	ComDCM.DCBlength = sizeof(DCB);
 	GetCommState(m_Handle, &ComDCM);
 	ComDCM.BaudRate = DWORD(baudrate);
 	ComDCM.ByteSize = 8;
 	ComDCM.Parity = NOPARITY;
 	ComDCM.StopBits = ONESTOPBIT;
 	ComDCM.fAbortOnError = TRUE;
 	ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
 	ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
 	ComDCM.fBinary = TRUE;
 	ComDCM.fParity = FALSE;
 	ComDCM.fInX = FALSE;
        ComDCM.fOutX = FALSE;
 	ComDCM.XonChar = 0;
 	ComDCM.XoffChar = (unsigned char)0xFF;
 	ComDCM.fErrorChar = FALSE;
 	ComDCM.fNull = FALSE;
 	ComDCM.fOutxCtsFlow = FALSE;
 	ComDCM.fOutxDsrFlow = FALSE;
 	ComDCM.XonLim = 128;
 	ComDCM.XoffLim = 128;

 	if(!SetCommState(m_Handle, &ComDCM)) {
 		CloseHandle(m_Handle);
 		m_Handle = INVALID_HANDLE_VALUE;
 		throw TTYException();
 	}

 }

 void TTY::Disconnect() {

 	if(m_Handle != INVALID_HANDLE_VALUE)
        {
 	   CloseHandle(m_Handle);
           m_Handle = INVALID_HANDLE_VALUE;
 	}

 }

 void TTY::Write(const vector<unsigned char>& data) {

 	if(m_Handle == INVALID_HANDLE_VALUE) {
 		throw TTYException();
 	}

 	DWORD feedback;
 	if(!WriteFile(m_Handle, &data[0], (DWORD)data.size(), &feedback, 0) || feedback != (DWORD)data.size()) {
 		CloseHandle(m_Handle);
 		m_Handle = INVALID_HANDLE_VALUE;
 		throw TTYException();
 	}

 	// In some cases it's worth uncommenting
 	//FlushFileBuffers(m_Handle);

 }

 void TTY::Read(vector<unsigned char>& data) {

 	if(m_Handle == INVALID_HANDLE_VALUE) {
 		throw TTYException();
 	}

 	DWORD begin = GetTickCount();
 	DWORD feedback = 0;

 	unsigned char* buf = &data[0];
 	DWORD len = (DWORD)data.size();

 	int attempts = 3;
 	while(len && (attempts || (GetTickCount()-begin) < (DWORD)TIMEOUT/3)) {

 		if(attempts) attempts--;

 		if(!ReadFile(m_Handle, buf, len, &feedback, NULL)) {
 			CloseHandle(m_Handle);
 			m_Handle = INVALID_HANDLE_VALUE;
 			throw TTYException();
 		}

 		assert(feedback <= len);
 		len -= feedback;
 		buf += feedback;

 	}

 	if(len) {
 		CloseHandle(m_Handle);
 		m_Handle = INVALID_HANDLE_VALUE;
 		throw TTYException();
 	}

 }

using namespace std;
TTY tty;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
    tty.Connect("COM2",CBR_115200);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
vector <unsigned char> Data;
 for(int i = 0; i<Edit1->Text.Length(); i++)
Data.push_back(Edit1->Text[i+1]);
    tty.Write(Data);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Close(TObject *Sender, TCloseAction &Action)
{
 tty.~TTY();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{



      Timer1->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
static vector<unsigned char> data;

/* Создаем объект синхронизации */
      sync.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

  if(SetCommMask(tty.m_Handle, EV_RXCHAR)) {    /* Устанавливаем маску на события порта */
  /* Связываем порт и объект синхронизации*/
	WaitCommEvent(tty.m_Handle, &state, &sync);
	/* Начинаем ожидание данных*/
	wait = WaitForSingleObject(sync.hEvent, READ_TIME);
	/* Данные получены */
	if(wait == WAIT_OBJECT_0) {
		/* Начинаем чтение данных */
                	if(GetOverlappedResult(tty.m_Handle, &sync, &read, FALSE))
               data.push_back(read);
               tty.Read(data);
		/* Ждем завершения операции чтения */
		wait = WaitForSingleObject(sync.hEvent, READ_TIME);
		/* Если все успешно завершено, узнаем какой объем данных прочитан */
                                Label1->Caption = Label1->Caption + (char)data[0];
	}
  }
 CloseHandle(sync.hEvent);
}
//---------------------------------------------------------------------------


