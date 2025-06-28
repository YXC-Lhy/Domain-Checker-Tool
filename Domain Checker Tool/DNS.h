#pragma once

#include "define.h"

#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <tlhelp32.h>


// ����Ͽ�����ӳ�䵽 nslookup �� qt ����
static const char* dnsTypeMap[] = {
    "a",     // A
    "aaaa",  // AAAA
    "cname", // CNAME
    "mx",    // MX
    "ns",    // NS
    "ptr",   // PTR
    "soa",   // SOA
    "txt",   // TXT
    "srv",   // SRV
};


//���DNS
std::wstring GetDNSRecords(HWND hWnd, const std::string& url){
    HWND hCombo = GetDlgItem(hWnd, COMBOBOX_1);
    int selectedIndex = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
    //if (selectedIndex == CB_ERR || selectedIndex >= sizeof(dnsTypeMap) / sizeof(dnsTypeMap[0]))
    //    return "";

    const char* dnsType = dnsTypeMap[selectedIndex];

    std::string command = "nslookup -qt=" + std::string(dnsType) + " " + url;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hChildStdoutRd, hChildStdoutWr;
    CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0);

    // �����ӽ���������Ϣ
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hChildStdoutWr;
    si.hStdError = hChildStdoutWr;
    si.wShowWindow = SW_HIDE; // ���ؿ���̨����

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // ת��Ϊ���ַ�����
    size_t commandLen = command.length();
    wchar_t* cmdLine = new wchar_t[commandLen + 1];
    MultiByteToWideChar(CP_ACP, 0, command.c_str(), -1, cmdLine, (int)(commandLen + 1));

    BOOL bSuccess = CreateProcess(
        NULL,
        cmdLine,           // ������
        NULL,              // ���̾�����ɼ̳�
        NULL,              // �߳̾�����ɼ̳�
        TRUE,              // �̳о��
        CREATE_NO_WINDOW,  // ����������
        NULL,              // ʹ�ø����̻���
        NULL,              // ʹ�ø�����Ŀ¼
        &si,               // ������Ϣ
        &pi                // ������Ϣ
    );

    delete[] cmdLine;

    if (!bSuccess)
        //return "";

    CloseHandle(hChildStdoutWr); // �ر�д���

    // ��ȡ���
    DWORD dwRead;
    CHAR chBuf[4096];
    std::ostringstream oss;
    while (true)
    {
        DWORD available = 0;
        if (!PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &available, NULL))
            break; // �ܵ������ر�

        if (available == 0)
        {
            // û���������ˣ��������Ƿ����
            DWORD exitCode;
            if (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE)
            {
                break; // ���̽������޸�������
            }

            Sleep(10);
            continue;
        }

        ZeroMemory(chBuf, sizeof(chBuf));
        if (!ReadFile(hChildStdoutRd, chBuf, sizeof(chBuf) - 1, &dwRead, NULL))
            break;

        chBuf[dwRead] = '\0';
        oss << chBuf;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hChildStdoutRd);

    std::string resultGBK = oss.str();

    // ת��Ϊ Unicode
    int len = MultiByteToWideChar(CP_ACP, 0, resultGBK.c_str(), -1, NULL, 0);
    std::wstring wtext(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, resultGBK.c_str(), -1, &wtext[0], len);

    return wtext;
}