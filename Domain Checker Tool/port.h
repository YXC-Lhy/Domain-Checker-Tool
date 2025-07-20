#pragma once

#include <windows.h>
#include <string>
#include <sstream>
#include <vector>

int TestTcpPort(HWND Hip,HWND Hport) {
    // 获取窗口文本长度
    int len = GetWindowTextLength(Hip);
    if (len <= 0) return FALSE;

    // 获取窗口文本
    std::vector<wchar_t> buffer(len + 1);
    GetWindowText(Hip, buffer.data(), len + 1);
    std::wstring host(buffer.data());

    // 获取窗口文本长度
    int len2 = GetWindowTextLength(Hport);
    if (len2 <= 0) return FALSE;

    // 获取窗口文本
    std::vector<wchar_t> buffer2(len2 + 1);
    GetWindowText(Hport, buffer2.data(), len2 + 1);
    std::wstring portStr(buffer2.data());

    /*// 分割 IP/域名 和端口
    size_t colonPos = text.find(L':');
    if (colonPos == std::wstring::npos || colonPos == 0 || colonPos == text.length() - 1)
        return FALSE;

    std::wstring host = text.substr(0, colonPos);
    std::wstring portStr = text.substr(colonPos + 1);*/

    // 检查端口是否为数字
    wchar_t* endPtr;
    long port = wcstol(portStr.c_str(), &endPtr, 10);
    if (*endPtr != L'\0' || port < 1 || port > 65535)
        return FALSE;

    // 构造 PowerShell 命令
    std::wstring cmd = L"powershell.exe -Command \"Test-NetConnection ";
    cmd += host;
    cmd += L" -Port ";
    cmd += portStr;
    cmd += L" | Select-Object -ExpandProperty TcpTestSucceeded\"";

    // 启动进程信息
    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // 创建管道
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hRead, &hWrite, &sa, 0);

    // 设置标准输出重定向
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // 启动 PowerShell
    if (!CreateProcessW(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return FALSE;
    }

    //WaitForSingleObject(pi.hProcess, INFINITE);
    // 
    // 等待进程结束，最多 5 秒 
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 5000);
    if (waitResult != WAIT_OBJECT_0) {
        // 超时，终止进程
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(hRead);
        CloseHandle(hWrite);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 3;
    }

    // 读取输出
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

    //MessageBoxA(NULL, output.c_str(), "PowerShell 输出", MB_OK);

    if (!output.empty() && output.find("True") != std::string::npos) {
        return TRUE;
    }
    return FALSE;

    // 判断输出是否为 "True"
    if (output.find("True") != std::string::npos) {
        return TRUE;
    }

    return FALSE;
}