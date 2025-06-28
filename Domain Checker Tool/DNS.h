#pragma once

#include "define.h"

#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <tlhelp32.h>


// 将组合框索引映射到 nslookup 的 qt 参数
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


//获得DNS
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

    // 设置子进程启动信息
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hChildStdoutWr;
    si.hStdError = hChildStdoutWr;
    si.wShowWindow = SW_HIDE; // 隐藏控制台窗口

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // 转换为宽字符命令
    size_t commandLen = command.length();
    wchar_t* cmdLine = new wchar_t[commandLen + 1];
    MultiByteToWideChar(CP_ACP, 0, command.c_str(), -1, cmdLine, (int)(commandLen + 1));

    BOOL bSuccess = CreateProcess(
        NULL,
        cmdLine,           // 命令行
        NULL,              // 进程句柄不可继承
        NULL,              // 线程句柄不可继承
        TRUE,              // 继承句柄
        CREATE_NO_WINDOW,  // 不创建窗口
        NULL,              // 使用父进程环境
        NULL,              // 使用父进程目录
        &si,               // 启动信息
        &pi                // 进程信息
    );

    delete[] cmdLine;

    if (!bSuccess)
        //return "";

    CloseHandle(hChildStdoutWr); // 关闭写入端

    // 读取输出
    DWORD dwRead;
    CHAR chBuf[4096];
    std::ostringstream oss;
    while (true)
    {
        DWORD available = 0;
        if (!PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &available, NULL))
            break; // 管道出错或关闭

        if (available == 0)
        {
            // 没有新数据了，检查进程是否结束
            DWORD exitCode;
            if (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE)
            {
                break; // 进程结束，无更多数据
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

    // 转换为 Unicode
    int len = MultiByteToWideChar(CP_ACP, 0, resultGBK.c_str(), -1, NULL, 0);
    std::wstring wtext(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, resultGBK.c_str(), -1, &wtext[0], len);

    return wtext;
}