#pragma once

#include <windows.h>
#include <string>
#include <sstream>
#include <vector>

int TestTcpPort(HWND Hip,HWND Hport) {
    // ��ȡ�����ı�����
    int len = GetWindowTextLength(Hip);
    if (len <= 0) return FALSE;

    // ��ȡ�����ı�
    std::vector<wchar_t> buffer(len + 1);
    GetWindowText(Hip, buffer.data(), len + 1);
    std::wstring host(buffer.data());

    // ��ȡ�����ı�����
    int len2 = GetWindowTextLength(Hport);
    if (len2 <= 0) return FALSE;

    // ��ȡ�����ı�
    std::vector<wchar_t> buffer2(len2 + 1);
    GetWindowText(Hport, buffer2.data(), len2 + 1);
    std::wstring portStr(buffer2.data());

    /*// �ָ� IP/���� �Ͷ˿�
    size_t colonPos = text.find(L':');
    if (colonPos == std::wstring::npos || colonPos == 0 || colonPos == text.length() - 1)
        return FALSE;

    std::wstring host = text.substr(0, colonPos);
    std::wstring portStr = text.substr(colonPos + 1);*/

    // ���˿��Ƿ�Ϊ����
    wchar_t* endPtr;
    long port = wcstol(portStr.c_str(), &endPtr, 10);
    if (*endPtr != L'\0' || port < 1 || port > 65535)
        return FALSE;

    // ���� PowerShell ����
    std::wstring cmd = L"powershell.exe -Command \"Test-NetConnection ";
    cmd += host;
    cmd += L" -Port ";
    cmd += portStr;
    cmd += L" | Select-Object -ExpandProperty TcpTestSucceeded\"";

    // ����������Ϣ
    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // �����ܵ�
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hRead, &hWrite, &sa, 0);

    // ���ñ�׼����ض���
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // ���� PowerShell
    if (!CreateProcessW(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return FALSE;
    }

    //WaitForSingleObject(pi.hProcess, INFINITE);
    // 
    // �ȴ����̽�������� 5 �� 
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 5000);
    if (waitResult != WAIT_OBJECT_0) {
        // ��ʱ����ֹ����
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(hRead);
        CloseHandle(hWrite);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 3;
    }

    // ��ȡ���
    CloseHandle(hWrite);
    char bufferOut[1024];
    DWORD bytesRead;
    std::string output;

    while (ReadFile(hRead, bufferOut, sizeof(bufferOut) - 1, &bytesRead, NULL) && bytesRead > 0) {
        bufferOut[bytesRead] = '\0';
        output += bufferOut;
    }

    CloseHandle(hRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    //MessageBoxA(NULL, output.c_str(), "PowerShell ���", MB_OK);

    if (!output.empty() && output.find("True") != std::string::npos) {
        return TRUE;
    }
    return FALSE;

    // �ж�����Ƿ�Ϊ "True"
    if (output.find("True") != std::string::npos) {
        return TRUE;
    }

    return FALSE;
}