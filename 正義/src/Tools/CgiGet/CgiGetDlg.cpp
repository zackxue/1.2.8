
// CgiGetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CgiGet.h"
#include "CgiGetDlg.h"

#include "curl.h"
#include "json.h"

#include "convert.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace Json;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCgiGetDlg dialog




CCgiGetDlg::CCgiGetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCgiGetDlg::IDD, pParent), 
    m_iUrlSize(2)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    Url url1 = {_T("10.1.21.34"), 80, _T("root"), _T("pass"), _T("info")};
    Url url2 = {_T("10.1.21.37"), 80, _T("root"), _T("pass"), _T("info")};
    m_UrlList[0] = url1;
    m_UrlList[1] = url2;

    //Get exec path
    GetModuleFileName(NULL, m_szExecPath, MAX_PATH);
    (_tcsrchr(m_szExecPath, _T('\\')))[1] = 0;//delete filename

    TCHAR szFile[MAX_PATH + 1]={0};
    FILE *file = NULL;
    std::ifstream fin;
    CHAR *chResponse;

    _stprintf_s(szFile, MAX_PATH, _T("%s\%s"), m_szExecPath, _T("response.json"));
    fin.open(szFile);

    if(!fin.is_open())    
        return;

    // get file size 
    fin.seekg(0,ifstream::end); 
    int size=fin.tellg(); 
    fin.seekg(0); 
    chResponse = new CHAR[size];
    ZeroMemory(chResponse, size);

    fin.read(chResponse, size);
    fin.close();

    Json::Reader reader;
    Json::Value root;

	USES_CONVERSION;

    if(reader.parse(chResponse, root))
    {
        int index = 0;
        for(Json::Value::iterator it = root.begin(); it != root.end(); it++)
        {
            string key = it.key().asString();
            string szValue = root[key.c_str()].asString();

            TCHAR url[10];
            TCHAR *response = NULL;

			
			url = A2W(key.c_str());
			//W2A

            //C2T(&url, key.c_str());
            //C2T(&response, szValue.c_str());

            _stscanf(url, _T("http://%[^:]:%[^@]@%[^:]:%d/config/%[^.].cgi"), 
                m_UrlList[index].Username, 
                m_UrlList[index].Password, 
                m_UrlList[index].IpAddr, 
                &m_UrlList[index].Port, 
                m_UrlList[index].Info);

            _tcscpy(m_UrlList[index].Response, response);

            delete url;
            delete response;

            index++;
        }

    }

    delete[] chResponse;

}

void CCgiGetDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_EDIT_IP1, m_UrlList[0].IpAddr, 1024);
    DDV_MaxChars(pDX, m_UrlList[0].IpAddr, 1024);
    DDX_Text(pDX, IDC_EDIT_PORT1, m_UrlList[0].Port);
    DDV_MinMaxDWord(pDX, m_UrlList[0].Port, 1, 9999);
    DDX_Text(pDX, IDC_EDIT_PASSWORD1, m_UrlList[0].Password, 16);
    DDV_MaxChars(pDX, m_UrlList[0].Password, 16);
    DDX_Text(pDX, IDC_EDIT_USERNAME1, m_UrlList[0].Username, 16);
    DDV_MaxChars(pDX, m_UrlList[0].Username, 16);
    DDX_Text(pDX, IDC_EDIT_INFO1, m_UrlList[0].Info, 255);
    DDV_MaxChars(pDX, m_UrlList[0].Info, 255);

    DDX_Text(pDX, IDC_EDIT_IP2, m_UrlList[1].IpAddr, 1024);
    DDV_MaxChars(pDX, m_UrlList[1].IpAddr, 1024);
    DDX_Text(pDX, IDC_EDIT_PORT2, m_UrlList[1].Port);
    DDV_MinMaxInt(pDX, m_UrlList[1].Port, 1, 9999);
    DDX_Text(pDX, IDC_EDIT_PASSWORD2, m_UrlList[1].Password, 16);
    DDV_MaxChars(pDX, m_UrlList[1].Password, 16);
    DDX_Text(pDX, IDC_EDIT_USERNAME2, m_UrlList[1].Username, 16);
    DDV_MaxChars(pDX, m_UrlList[1].Username, 16);
    DDX_Text(pDX, IDC_EDIT_INFO2, m_UrlList[1].Info, 255);
    DDV_MaxChars(pDX, m_UrlList[1].Info, 255);

}

BEGIN_MESSAGE_MAP(CCgiGetDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_GET, &CCgiGetDlg::OnBnClickedButtonGet)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CCgiGetDlg message handlers

BOOL CCgiGetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here



    CListCtrl *lstCtrl = (CListCtrl *)GetDlgItem(IDC_LIST_MSG);
    lstCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_GRIDLINES|LVS_EX_FLATSB);

    lstCtrl->InsertColumn(0, _T("IP"));
    lstCtrl->InsertColumn(1, _T("Key"));
    lstCtrl->InsertColumn(2, _T("Value"));

    lstCtrl->SetColumnWidth(0, 80);    
    lstCtrl->SetColumnWidth(1, 80);    
    lstCtrl->SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);

    for(int index = 0; index < m_iUrlSize; index++)
    {   

        CHAR *chUrl = NULL;
        CHAR *chResponse = NULL;
        TCHAR chTemp[1024];

        _stprintf(chTemp, _T("http://%s:%s@%s:%d/config/%s.cgi"), 
            m_UrlList[index].Username, m_UrlList[index].Password, m_UrlList[index].IpAddr, m_UrlList[index].Port, m_UrlList[index].Info);

        T2C(&chUrl, (const TCHAR *)chTemp);
        T2C(&chResponse, m_UrlList[index].Response);

        SetListCtrlItemsByJson(chResponse, chUrl);

        delete chUrl;
        delete chResponse;
    }

    
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCgiGetDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCgiGetDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCgiGetDlg::OnBnClickedButtonGet()
{
    // TODO: Add your control notification handler code here
    UpdateData();

    int index = 0;
    CListCtrl *lstCtrl = (CListCtrl *)GetDlgItem(IDC_LIST_MSG);

    m_csResponse.clear();
    lstCtrl->SetRedraw(FALSE);  
    lstCtrl->DeleteAllItems();
    for(index = 0; index < m_iUrlSize; index++)
    {
        GetCgiResponse(m_UrlList[index]);    
    }
    lstCtrl->SetRedraw(TRUE);  

    TCHAR szFile[MAX_PATH + 1]={0};
    FILE *file = NULL;
    std::ofstream fout;

    _stprintf(szFile, _T("%s\%s"), m_szExecPath, _T("response.json"));
    fout.open(szFile);
    fout << m_csResponse.toStyledString();
    fout.close();

}

void CCgiGetDlg::GetCgiResponse(Url url)
{

    CString csURL;
    CString csPort;
    CString csResponse;
    csPort.Format(_T("%d"),url.Port);

    if (url.Username[0] == 0 || url.Password[0] == 0)
    {
        csURL.Format(_T("http://%s:%d/config/%s.cgi"), url.IpAddr, url.Port, url.Info);
    }
    else
    {
        csURL.Format(_T("http://%s:%s@%s:%d/config/%s.cgi"), url.Username, url.Password, url.IpAddr, url.Port, url.Info);
    }

    char *chUrl = new char[csURL.GetLength() + 1];
    wcstombs_s(NULL, chUrl, csURL.GetLength() + 1, csURL, _TRUNCATE);

    curl_global_init(CURL_GLOBAL_ALL);

    CURL *curl = curl_easy_init();
    CURLcode res;
    long retcode = 0;



#ifdef _DEBUG
    res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif

    res = curl_easy_setopt(curl, CURLOPT_URL, chUrl);

    res = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 

    res = curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 20000); 

    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &csResponse); 

    res = curl_easy_perform(curl);
    if(res == CURLE_OK)
    {
        
        char *chResponse = new char[csResponse.GetLength() + 1];
        wcstombs_s(NULL, chResponse, csResponse.GetLength() + 1, csResponse, _TRUNCATE);

        if(!SetListCtrlItemsByJson(chResponse, chUrl))
        {             
            res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode); 
            TCHAR msg[200];
            _stprintf(msg, _T("HTTP status code : %d"), retcode);
            m_csResponse[chUrl] = string("error");
            MessageBox(msg, url.IpAddr,MB_OK);
        }
        delete chResponse;
    }
    else
    {
        TCHAR msg[200];
        _stprintf(msg, _T("curl errorcode : %d"), res);
        m_csResponse[chUrl] = string("error");
        MessageBox(msg, url.IpAddr, MB_OK);
    }
    curl_easy_cleanup(curl);
    delete chUrl;
}

size_t CCgiGetDlg::write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    CString *csJson = (CString *)stream; 
    csJson->Append(CString((char *)ptr));
    return size * nmemb;
}


bool CCgiGetDlg::SetListCtrlItemsByJson(const char *chResponse, const char *chUrl)
{
    bool bResult;
    Json::Value jsValue; 
    Json::Reader jsReader;
    string szResponse = string(chResponse);

    CHAR chIpAddr[1024];
    DWORD nPort;
    CHAR chUsername[16];
    CHAR chPassword[16];
    CHAR chInfo[255];

    sscanf(chUrl, "http://%[^:]:%[^@]@%[^:]:%d/config/%[^.].cgi", chUsername, chPassword, chIpAddr, &nPort, chInfo );

    bResult = jsReader.parse(szResponse, jsValue);
    if(!bResult)
        return bResult;
 
    CListCtrl *lstCtrl = (CListCtrl *)GetDlgItem(IDC_LIST_MSG);

    int count = 0;
    Json::Value pair;
    m_csResponse[chUrl] = szResponse;

    Json::Value::Members members( jsValue.getMemberNames() );
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        CString csValue;

        const std::string &szKey = *it;
        TCHAR *chKey ;
        TCHAR *chValue ;
        TCHAR *chIP;

        C2T(&chKey, szKey.c_str());
        C2T(&chValue, jsValue[szKey].asCString());
        C2T(&chIP, chIpAddr);

        int nItem = lstCtrl->InsertItem(count, chIP);
        lstCtrl->SetItemText(nItem, 1, CString(chKey));
        lstCtrl->SetItemText(nItem, 2, CString(chValue));

        delete chKey;
        delete chValue;
        delete chIP;
        count++;
    }

    return bResult;
}
//
//void AFXAPI CCgiGetDlg::DDX_TextByJson(CDataExchange* pDX, int nIDC, Json::Value& value, Json::ValueType _type)
//{
//    HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
//    CString csValue;
//    if (pDX->m_bSaveAndValidate)
//    {
//        
//        int nLen = ::GetWindowTextLength(hWndCtrl);
//        ::GetWindowText(hWndCtrl, csValue.GetBufferSetLength(nLen), nLen+1);
//        csValue.ReleaseBuffer();
//    }
//    else
//    {
//        AfxSetWindowText(hWndCtrl, csValue);
//    
//        switch(_type)
//        {
//        case Json::stringValue:
//            //std::string szValue(T2C(csValue.GetString()));
//            //value = szValue;
//            break;
//        }
//    }
//}
