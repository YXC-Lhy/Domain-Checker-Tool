#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
#include <thread>
#include <chrono>

struct PingResult {
    std::string ip;
    int ttl = -1;
    std::vector<int> delays; // 延迟时间（ms）
};

PingResult PingWebsite(const std::string& url) {
    PingResult result;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hPipeRead = nullptr;
    HANDLE hPipeWrite = nullptr;

    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        std::cerr << "创建管道失败: " << GetLastError() << "\n";
        return result;
    }

    if (!SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed\n";
        CloseHandle(hPipeRead);
        CloseHandle(hPipeWrite);
        return result;
    }

    std::string commandLine = "ping -n 4 " + url;

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;

    PROCESS_INFORMATION pi;

    char* cmdLine = new char[commandLine.size() + 1];
    strcpy_s(cmdLine, commandLine.size() + 1, commandLine.c_str());

    BOOL success = CreateProcessA(
        nullptr,
        cmdLine,
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    delete[] cmdLine;

    if (!success) {
        std::cerr << "创建进程失败: " << GetLastError() << "\n";
        CloseHandle(hPipeRead);
        CloseHandle(hPipeWrite);
        return result;
    }

    CloseHandle(hPipeWrite);

    // Read output
    const int BUF_SIZE = 4096;
    char buffer[BUF_SIZE];
    DWORD bytesRead;
    std::string output;

    // Wait for process to finish (up to 15 seconds)
    WaitForSingleObject(pi.hProcess, 20000);  // 最多等20秒

    while (ReadFile(hPipeRead, buffer, BUF_SIZE, &bytesRead, nullptr) && bytesRead > 0) {
        output.append(buffer, bytesRead);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hPipeRead);

    // Debug: 打印原始输出
    // std::cout << "原始输出:\n" << output << "\n";
    

    // 提取 IP
    std::regex ipRegex(R"(\b(?:\d{1,3}\.){3}\d{1,3}\b)");
    std::smatch match;
    if (std::regex_search(output, match, ipRegex)) {
        result.ip = match.str();
    }

    // 提取 TTL
    std::regex ttlRegex(R"(TTL=(\d+))", std::regex::icase);
    if (std::regex_search(output, match, ttlRegex)) {
        try {
            result.ttl = std::stoi(match[1].str());
        }
        catch (...) {}
    }

    // 提取每次 ping 的延迟（最多 4 次），没有则补 0
    std::regex timeRegex(R"(时间[=<](\d+)ms)", std::regex::icase);
    auto begin = std::sregex_iterator(output.begin(), output.end(), timeRegex);
    auto end = std::sregex_iterator();

    std::vector<int> delays;
    for (int i = 0; i < 4; ++i) {
        if (begin != end && i < 4) {
            try {
                delays.push_back(std::stoi((*begin)[1].str()));
                ++begin;
            }
            catch (...) {
                delays.push_back(0);
            }
        }
        else {
            delays.push_back(-1); // 补 -1
        }
    }

    result.delays = std::move(delays);

    return result;
}



int stop[1000];//停止用变量，默认为0，设为1发送停止消息

int pingi;//累计时间变量
int pingi2;//累计变量
int pingi3[536698431];//存值变量
int pingi4;//非TO计数变量
int pingmax;
int pingmin;
int ThreadID = 0;

// 定义自定义消息 WM_UPDATE_UI
#define WM_PING_END (WM_USER + 1)

struct UpdateData {
    std::string ip;
    int ttl;
    std::vector<int> delays;
};

PingResult PingWebsite(const std::string& url);

//新线程执行一次Ping(父窗口句柄，url)
void PingOnce(HWND hWnd, const std::string& url) {
    pingi = 0;
    pingi2 = 4;
    pingi4 = 0;
    pingmax = 0;
    pingmin = 9999;
    ThreadID++;
    int THID = ThreadID;
    // 启动新线程执行 PingWebsite
    std::thread pingThread([=]() {
        PingResult res = PingWebsite(url);

        // 准备要发送的数据
        struct UpdateData {
            std::string ip;
            int ttl;
            std::vector<int> delays;
        };

        auto data = new UpdateData{ res.ip, res.ttl, res.delays };

        // 发送消息给主线程
        if (stop[THID] == 0)PostMessage(hWnd, WM_PING_END, reinterpret_cast<WPARAM>(data), 0);
        });
    
    // 确保线程结束后自动释放资源
    pingThread.detach();
    
}

//HANDLE hWorkerThread = nullptr; // 全局线程句柄
int sleeptime = 0;//间隔休息时长

//新线程重复执行Ping(父窗口句柄，url)
void PingInfinite(HWND hWnd, const std::string& url) {
    pingi = 0;
    pingi2 = 0;
    pingi4 = 0;
    pingmax = 0;
    pingmin = 9999;
    ThreadID++;
    int THID = ThreadID;
    stop[THID] = 0;
    // 启动新线程执行 PingWebsite
    std::thread pingThread([=]() {
        while(stop[THID] == 0){
            PingResult res = PingWebsite(url);
            if (stop[THID] == 0)pingi2 = pingi2 + 4;
            // 准备要发送的数据
            struct UpdateData {
                std::string ip;
                int ttl;
                std::vector<int> delays;
            };

            auto data = new UpdateData{ res.ip, res.ttl, res.delays };

            // 发送消息给主线程
            if(stop[THID] == 0)PostMessage(hWnd, WM_PING_END, reinterpret_cast<WPARAM>(data), 0);
            if (sleeptime > 0) {
                for (int i=0; i < sleeptime; i+=5) {
                    Sleep(5000);
                }
            }
        }
        //stop = 0;
    });
    
    // 获取线程句柄
    //hWorkerThread = pingThread.native_handle(); 
    
    // 确保线程结束后自动释放资源
    pingThread.detach();
    
}

std::wstringstream pingdetail[999];//ping的详细信息
int pingdetailpage;//ping的总页码
int pingdetailreadpage;//ping正在看的页码

