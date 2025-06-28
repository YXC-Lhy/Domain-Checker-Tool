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
    std::vector<int> delays; // �ӳ�ʱ�䣨ms��
};

PingResult PingWebsite(const std::string& url) {
    PingResult result;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hPipeRead = nullptr;
    HANDLE hPipeWrite = nullptr;

    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        std::cerr << "�����ܵ�ʧ��: " << GetLastError() << "\n";
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
        std::cerr << "��������ʧ��: " << GetLastError() << "\n";
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
    WaitForSingleObject(pi.hProcess, 20000);  // ����20��

    while (ReadFile(hPipeRead, buffer, BUF_SIZE, &bytesRead, nullptr) && bytesRead > 0) {
        output.append(buffer, bytesRead);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hPipeRead);

    // Debug: ��ӡԭʼ���
    // std::cout << "ԭʼ���:\n" << output << "\n";
    

    // ��ȡ IP
    std::regex ipRegex(R"(\b(?:\d{1,3}\.){3}\d{1,3}\b)");
    std::smatch match;
    if (std::regex_search(output, match, ipRegex)) {
        result.ip = match.str();
    }

    // ��ȡ TTL
    std::regex ttlRegex(R"(TTL=(\d+))", std::regex::icase);
    if (std::regex_search(output, match, ttlRegex)) {
        try {
            result.ttl = std::stoi(match[1].str());
        }
        catch (...) {}
    }

    // ��ȡÿ�� ping ���ӳ٣���� 4 �Σ���û���� 0
    std::regex timeRegex(R"(ʱ��[=<](\d+)ms)", std::regex::icase);
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
            delays.push_back(-1); // �� -1
        }
    }

    result.delays = std::move(delays);

    return result;
}



int stop[1000];//ֹͣ�ñ�����Ĭ��Ϊ0����Ϊ1����ֹͣ��Ϣ

int pingi;//�ۼ�ʱ�����
int pingi2;//�ۼƱ���
int pingi3[536698431];//��ֵ����
int pingi4;//��TO��������
int pingmax;
int pingmin;
int ThreadID = 0;

// �����Զ�����Ϣ WM_UPDATE_UI
#define WM_PING_END (WM_USER + 1)

struct UpdateData {
    std::string ip;
    int ttl;
    std::vector<int> delays;
};

PingResult PingWebsite(const std::string& url);

//���߳�ִ��һ��Ping(�����ھ����url)
void PingOnce(HWND hWnd, const std::string& url) {
    pingi = 0;
    pingi2 = 4;
    pingi4 = 0;
    pingmax = 0;
    pingmin = 9999;
    ThreadID++;
    int THID = ThreadID;
    // �������߳�ִ�� PingWebsite
    std::thread pingThread([=]() {
        PingResult res = PingWebsite(url);

        // ׼��Ҫ���͵�����
        struct UpdateData {
            std::string ip;
            int ttl;
            std::vector<int> delays;
        };

        auto data = new UpdateData{ res.ip, res.ttl, res.delays };

        // ������Ϣ�����߳�
        if (stop[THID] == 0)PostMessage(hWnd, WM_PING_END, reinterpret_cast<WPARAM>(data), 0);
        });
    
    // ȷ���߳̽������Զ��ͷ���Դ
    pingThread.detach();
    
}

//HANDLE hWorkerThread = nullptr; // ȫ���߳̾��
int sleeptime = 0;//�����Ϣʱ��

//���߳��ظ�ִ��Ping(�����ھ����url)
void PingInfinite(HWND hWnd, const std::string& url) {
    pingi = 0;
    pingi2 = 0;
    pingi4 = 0;
    pingmax = 0;
    pingmin = 9999;
    ThreadID++;
    int THID = ThreadID;
    stop[THID] = 0;
    // �������߳�ִ�� PingWebsite
    std::thread pingThread([=]() {
        while(stop[THID] == 0){
            PingResult res = PingWebsite(url);
            if (stop[THID] == 0)pingi2 = pingi2 + 4;
            // ׼��Ҫ���͵�����
            struct UpdateData {
                std::string ip;
                int ttl;
                std::vector<int> delays;
            };

            auto data = new UpdateData{ res.ip, res.ttl, res.delays };

            // ������Ϣ�����߳�
            if(stop[THID] == 0)PostMessage(hWnd, WM_PING_END, reinterpret_cast<WPARAM>(data), 0);
            if (sleeptime > 0) {
                for (int i=0; i < sleeptime; i+=5) {
                    Sleep(5000);
                }
            }
        }
        //stop = 0;
    });
    
    // ��ȡ�߳̾��
    //hWorkerThread = pingThread.native_handle(); 
    
    // ȷ���߳̽������Զ��ͷ���Դ
    pingThread.detach();
    
}

std::wstringstream pingdetail[999];//ping����ϸ��Ϣ
int pingdetailpage;//ping����ҳ��
int pingdetailreadpage;//ping���ڿ���ҳ��

