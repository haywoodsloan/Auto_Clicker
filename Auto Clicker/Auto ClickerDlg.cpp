
// Auto ClickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Auto Clicker.h"
#include "Auto ClickerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAutoClickerDlg dialog
CButton m_HotkeyBtn;
CEdit m_DelayEdit;
CStatic m_Indicator;

bool newShortcut = false;
bool *clicking = NULL;

DWORD shortcut = VK_F4;
HANDLE semaphore;
HBRUSH m_RedBrush = CreateSolidBrush(RGB(255, 0, 0));
HBRUSH m_GreenBrush = CreateSolidBrush(RGB(0, 255, 0));


CAutoClickerDlg::CAutoClickerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_AUTOCLICKER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoClickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOTKEYBTN, m_HotkeyBtn);
	DDX_Control(pDX, IDC_DELAY, m_DelayEdit);
	DDX_Control(pDX, IDC_INDICATOR, m_Indicator);
}

BEGIN_MESSAGE_MAP(CAutoClickerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_HOTKEYBTN, &CAutoClickerDlg::OnBnClickedHotkeybtn)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CAutoClickerDlg message handlers

struct threadParam {
	bool* active;
	HANDLE semaphore;
};

DWORD WINAPI clickThreadProc(LPVOID lParam) {
	threadParam *param = (threadParam*)lParam;

	int delayLength = m_DelayEdit.GetWindowTextLengthW();
	WCHAR *delayString = new WCHAR[delayLength + 1];
	m_DelayEdit.GetWindowTextW(delayString, delayLength + 1);

	int sleepDuration = _wtoi(delayString);
	delete[] delayString;

	while (*param->active) {
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		WaitForSingleObject(param->semaphore, sleepDuration);
	}

	delete param->active;
	CloseHandle(param->semaphore);
	delete param;

	return 0;
}

LRESULT CALLBACK LLKeyHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {
		if (wParam == WM_KEYDOWN) {
			KBDLLHOOKSTRUCT* khs = (KBDLLHOOKSTRUCT*)lParam;

			if (newShortcut) {
				shortcut = khs->vkCode;

				WCHAR buff[10];
				DWORD dwMsg = 0;
				dwMsg += khs->scanCode << 16;
				dwMsg += khs->flags << 24;

				GetKeyNameText(dwMsg, buff, 10);
				m_HotkeyBtn.SetWindowText(buff);

				newShortcut = false;
			}
			else if (khs->vkCode == shortcut) {

				if (clicking != NULL) {
					*clicking = false;
					clicking = NULL;

					ReleaseSemaphore(semaphore, 1, 0);

				}
				else {
					clicking = new bool(1);
					semaphore = CreateSemaphore(0, 0, 1, 0);

					threadParam *param = new threadParam{ clicking, semaphore };
					CreateThread(0, 0, clickThreadProc, LPVOID(param), 0, 0);
				}

				m_Indicator.RedrawWindow();
			}
		}
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

BOOL CAutoClickerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_DelayEdit.SetWindowTextW(L"10");
	SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyHook, 0, 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAutoClickerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAutoClickerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAutoClickerDlg::OnBnClickedHotkeybtn()
{
	newShortcut = true;
	m_HotkeyBtn.SetWindowText(L"...");
}



HBRUSH CAutoClickerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_INDICATOR) {
		return (clicking != NULL) ? m_GreenBrush : m_RedBrush;
	}
	return hbr;
}
