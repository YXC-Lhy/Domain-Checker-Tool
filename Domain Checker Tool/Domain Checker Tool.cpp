// Domain Checker Tool.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Domain Checker Tool.h"
#include "others.h"
#include "ping.h"
#include "time.h"
#include "DNS.h"

#include <cmath>
#include <thread>
#include <commctrl.h> // 包含Common Controls库的相关定义
#pragma comment(lib, "Comctl32.lib")//标签页使用


// 初始化Common Controls库
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING 100

#include "define.h"

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES; // 初始化标签页控件
    InitCommonControlsEx(&icex);
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DOMAINCHECKERTOOL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DOMAINCHECKERTOOL));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DOMAINCHECKERTOOL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DOMAINCHECKERTOOL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, //szTitle, 
       L"域名检测工具", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
      //CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
       200, 100, 600, 500, //x,y,w,h
       nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   hWndMain = hWnd;//设定主窗口句柄到全局

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}






//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hTabControl = NULL;//新建标签页
    
    switch (message)
    {
    
    case WM_PING_END:
    {
        // 获取从子线程传过来的数据
        auto data = reinterpret_cast<UpdateData*>(wParam);
        std::string ip = data->ip;
        int ttl = data->ttl;
        std::vector<int> delays = data->delays;

        HWND IP = GetDlgItem(hWnd, EDIT_IP); // 获取编辑控件句柄
        std::string newContent = ip;
        // 从 std::string 转换到 LPCWSTR (宽字符)
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, newContent.c_str(), newContent.length(), NULL, 0);
        std::wstring wstrNewContent(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, newContent.c_str(), newContent.length(), &wstrNewContent[0], size_needed);
        // 更新文本
        //SetWindowTextW(IP, wstrNewContent.c_str());
        UpdateStaticText(IP, wstrNewContent.c_str());

        HWND TTL = GetDlgItem(hWnd, EDIT_TTL); // 获取编辑控件句柄
        // 更新 TTL 到 EDIT_TTL 编辑框
        // 将 int 转换为 std::wstring
        std::wstringstream ws;
        ws << ttl;
        std::wstring wstrTTL = ws.str();
        // 更新TTL文本
        UpdateStaticText(TTL, wstrTTL.c_str());

        //更新延迟时间
        //计算平均值
        int sum = 0;
        int count = 0;
        for (int i = 0; i < 4; ++i) {
            if (delays[i] > 0) {
                sum += delays[i];
                ++count;
                if (delays[i] > pingmax)pingmax = delays[i];//最大
                if (delays[i] < pingmin)pingmin = delays[i];//最小
            }
        }
        pingi += sum;//累计延迟时间和
        pingi4 += count;//累计非TO个数
        if (pingi4 != 0) {
            HWND ms = GetDlgItem(hWnd, EDIT_ms1); // 获取控件句柄
            std::wstringstream ws2;
            ws2 << pingi / pingi4;//求平均值
            std::wstring wstrd2 = ws2.str();
            UpdateStaticText(ms, wstrd2.c_str());//更新平均值
            
            HWND msmax = GetDlgItem(hWnd, EDIT_msmax); // 获取控件句柄
            std::wstringstream ws3;
            ws3 << pingmax;
            std::wstring wstrd3 = ws3.str();
            UpdateStaticText(msmax, wstrd3.c_str());//更新最大值

            HWND msmin = GetDlgItem(hWnd, EDIT_msmin); // 获取控件句柄
            std::wstringstream ws4;
            ws4 << pingmin;
            std::wstring wstrd4 = ws4.str();
            UpdateStaticText(msmin, wstrd4.c_str());//更新最小值

            
        }
        HWND pc = GetDlgItem(hWnd, EDIT_percent); // 获取控件句柄
        std::wstringstream ws41;
        float a= pingi4 * 100 / pingi2;
        ws41 << round(a * 10) / 10;
        std::wstring wstrd41 = ws41.str();
        UpdateStaticText(pc, wstrd41.c_str());//更新百分比

        HWND pc2 = GetDlgItem(hWnd, EDIT_percent2); // 获取控件句柄
        std::wstringstream ws42;
        ws42 << 100 - a;//round(((pingi2 - pingi4) * 100 / pingi2)*10)/10;
        std::wstring wstrd42 = ws42.str();
        UpdateStaticText(pc2, wstrd42.c_str());//更新百分比

        HWND ci = GetDlgItem(hWnd, EDIT_ci); // 获取控件句柄
        std::wstringstream ws5;
        ws5 << pingi2;
        std::wstring wstrd5 = ws5.str();
        UpdateStaticText(ci, wstrd5.c_str());//更新次数

        HWND ci1 = GetDlgItem(hWnd, EDIT_ci1); // 获取控件句柄
        std::wstringstream ws6;
        ws6 << pingi4;
        std::wstring wstrd6 = ws6.str();
        UpdateStaticText(ci1, wstrd6.c_str());//更新次数

        HWND ci2 = GetDlgItem(hWnd, EDIT_ci2); // 获取控件句柄
        std::wstringstream ws7;
        ws7 << pingi2 - pingi4;
        std::wstring wstrd7 = ws7.str();
        UpdateStaticText(ci2, wstrd7.c_str());//更新次数

        /*// 分配空间并获取文本
        int len = GetWindowTextLength(slk2);
        std::wstring text(len, L'\0');
        GetWindowTextW(slk2, &text[0], len + 1); // +1 包含结尾的 \0*/

        // 构造显示消息
        if (pingi2 > pingdetailpage * 2000) {
            pingdetailpage++;//加页
            std::wstringstream ws;
            ws << pingdetailpage;
            std::wstring wstrd = ws.str();
            HWND page = GetDlgItem(hWnd, EDIT_pageNO); // 获取控件句柄
            UpdateStaticText(page, wstrd.c_str());//更新
            if (pingdetailreadpage == pingdetailpage - 1) {
                pingdetailreadpage++;//加页
                std::wstringstream ws2;
                ws2 << pingdetailreadpage;
                std::wstring wstrd2 = ws2.str();
                HWND readpage = GetDlgItem(hWnd, EDIT_pagereadNO); // 获取控件句柄
                UpdateStaticText(readpage, wstrd2.c_str());//更新
            }
        }
        pingdetail[pingdetailpage] << GetTime();
        pingdetail[pingdetailpage] << "  " << std::wstring(ip.begin(), ip.end());
        //oss << " TTL:" << ttl;
        for (size_t i = 0; i < delays.size(); ++i) {
            if (delays[i] > 0)pingdetail[pingdetailpage] << "  " << delays[i] << " ms";
            else pingdetail[pingdetailpage] << "  " << "Timeout";
        }
        pingdetail[pingdetailpage] << "\r\n";
        if (pingdetailreadpage == pingdetailpage) {
            std::wstring woss = pingdetail[pingdetailpage].str();
            HWND slk2 = GetDlgItem(hWnd, EDIT_slk2); // 获取控件句柄
            UpdateStaticText(slk2, woss.c_str());//更新详细信息
        }
        /*
        ////debug////
        // 构造显示消息
        std::ostringstream oss;
        oss << "IP: " << ip << "\n";
        oss << "TTL: " << ttl << "\n";
        oss << "延迟 (ms):\n";
        for (size_t i = 0; i < delays.size(); ++i) {
            oss << "  Delay " << i + 1 << ": " << delays[i] << " ms\n";
        }
        // 显示debug信息
        MessageBoxA(nullptr, oss.str().c_str(), "Ping Debug Result", MB_ICONINFORMATION | MB_OK);
        */
        HWND checkBox = GetDlgItem(hWnd, IDM_checkBox);
        if (SendMessage(checkBox, BM_GETCHECK, 0, 0) == BST_CHECKED)
        {
            //持续测试无需操作
        }
        else {
            //测试一次
            HWND ping = GetDlgItem(hWnd, IDM_ping);
            EnableWindow(ping, TRUE); // ping按钮恢复（可点击）
            HWND checkBox = GetDlgItem(hWnd, IDM_checkBox);
            EnableWindow(checkBox, TRUE); // 按钮恢复（可点击）
            HWND wenzi2 = GetDlgItem(hWnd, EDIT_wenzi2);
            EnableWindow(wenzi2, TRUE); // 文字恢复
            HWND stopbutton = GetDlgItem(hWnd, IDM_stop);
            EnableWindow(stopbutton, FALSE); // 按钮禁用
            HWND slk = GetDlgItem(hWnd, EDIT_slk1);
            EnableWindow(slk, TRUE); // 启用编辑框
        }
        // 清理分配的内存
        delete data;
        break;
    }
    case WM_NCLBUTTONDBLCLK: // 双击非客户区（例如标题栏）
        {
        // 双击的是标题栏，则返回0以阻止默认行为
        //if (wParam == HTCAPTION)
        {
            return 0;
        }
        }
        break;
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;

        HWND hwndCtrl = (HWND)lParam;
        UINT ctrlType = GetWindowLong(hwndCtrl, GWL_STYLE) & 0xFF;

        if (ctrlType == ES_AUTOHSCROLL)//是编辑框
        {
            return (LONG)GetStockObject(WHITE_BRUSH);//默认画刷
        }
        else{//是静态文本
            //SetTextColor(hdcStatic, RGB(0, 0, 0)); // 设置文本颜色为黑色
            //SetBkMode(hdcStatic, TRANSPARENT); // 设置背景模式为透明
            //返回空画刷，以确保不绘制背景
            return (LONG)GetStockObject(HOLLOW_BRUSH);
        }
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_32771:
                CaptureWindowToFile(hWnd);
                break;
            case IDM_ping:
            {
                //stop = 0;//stop归零，防止出错

                pingdetailpage = 1;
                pingdetailreadpage = 1;//页码归零
                HWND page = GetDlgItem(hWnd, EDIT_pageNO); // 获取控件句柄
                UpdateStaticText(page, L"1");//更新
                HWND readpage = GetDlgItem(hWnd, EDIT_pagereadNO); // 获取控件句柄
                UpdateStaticText(readpage, L"1");//更新
                for (auto& ws : pingdetail) {
                    ws.str(L"");     // 清除内容
                    ws.clear();      // 重置流的状态标志
                }

                wchar_t buffer[256] = { 0 }; // 宽字符缓冲区
                HWND slk = GetDlgItem(hWnd, EDIT_slk1);//获取EDIT控件的句柄
                GetWindowTextW(slk, buffer, 256); // 从窗口获取文本

                // 将宽字符转为 std::string
                int len = wcslen(buffer);
                std::string url(len * 2, 0);

                WideCharToMultiByte(CP_ACP, 0, buffer, -1, &url[0], (int)url.size(), nullptr, nullptr);
                url.resize(strlen(url.c_str())); // 去掉多余部分
                
                HWND checkBox = GetDlgItem(hWnd, IDM_checkBox);
                if (SendMessage(checkBox, BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    //持续测试
                    PingInfinite(hWnd, url);//执行函数
                }
                else {
                    //测试一次
                    PingOnce(hWnd, url);//执行函数
                }

                HWND ip = GetDlgItem(hWnd, EDIT_IP); // 获取控件句柄
                UpdateStaticText(ip, L"---.---.---.---");//更新IP
                HWND ttl = GetDlgItem(hWnd, EDIT_TTL); // 获取控件句柄
                UpdateStaticText(ttl, L"----");//更新IP
                HWND ms = GetDlgItem(hWnd, EDIT_ms1); // 获取控件句柄
                UpdateStaticText(ms, L"----");//更新平均值
                HWND msmax = GetDlgItem(hWnd, EDIT_msmax); // 获取控件句柄
                UpdateStaticText(msmax, L"----");//更新最大值
                HWND msmin = GetDlgItem(hWnd, EDIT_msmin); // 获取控件句柄
                UpdateStaticText(msmin, L"----");//更新最小值
                HWND ci = GetDlgItem(hWnd, EDIT_ci); // 获取控件句柄
                UpdateStaticText(ci, L"----");//更新次数
                HWND ci1 = GetDlgItem(hWnd, EDIT_ci1); // 获取控件句柄
                UpdateStaticText(ci1, L"----");//更新次数
                HWND ci2 = GetDlgItem(hWnd, EDIT_ci2); // 获取控件句柄
                UpdateStaticText(ci2, L"----");//更新次数
                HWND pc = GetDlgItem(hWnd, EDIT_percent); // 获取控件句柄
                UpdateStaticText(pc, L"---");//更新百分比
                HWND pc2 = GetDlgItem(hWnd, EDIT_percent2); // 获取控件句柄
                UpdateStaticText(pc2, L"---");//更新百分比
                HWND slk2 = GetDlgItem(hWnd, EDIT_slk2); // 获取控件句柄
                UpdateStaticText(slk2, L"");//更新百分比

                HWND ping = GetDlgItem(hWnd, IDM_ping);
                EnableWindow(ping, FALSE); // ping按钮变灰（不可点击）
                //HWND checkBox = GetDlgItem(hWnd, IDM_checkBox);
                EnableWindow(checkBox, FALSE); // 按钮变灰（不可点击）
                HWND wenzi2 = GetDlgItem(hWnd, EDIT_wenzi2);
                EnableWindow(wenzi2, FALSE); // 文字变灰
                HWND stopbutton= GetDlgItem(hWnd, IDM_stop);
                EnableWindow(stopbutton, TRUE); // 按钮启用
                //HWND slk = GetDlgItem(hWnd, EDIT_slk1);
                EnableWindow(slk, FALSE); // 禁用编辑框
                
                break;
            }
            case IDM_backpage:
                if (pingdetailreadpage > 1) {
                    pingdetailreadpage--;
                    std::wstring woss = pingdetail[pingdetailreadpage].str();
                    HWND slk2 = GetDlgItem(hWnd, EDIT_slk2); // 获取控件句柄
                    UpdateStaticText(slk2, woss.c_str());//更新详细信息
                    std::wstringstream ws;
                    ws << pingdetailreadpage;
                    std::wstring wstrd = ws.str();
                    HWND page = GetDlgItem(hWnd, EDIT_pagereadNO); // 获取控件句柄
                    UpdateStaticText(page, wstrd.c_str());//更新
                }
                break;
            case IDM_nextpage:
                if (pingdetailpage > pingdetailreadpage) {
                    pingdetailreadpage++;
                    std::wstring woss = pingdetail[pingdetailreadpage].str();
                    HWND slk2 = GetDlgItem(hWnd, EDIT_slk2); // 获取控件句柄
                    UpdateStaticText(slk2, woss.c_str());//更新详细信息
                    std::wstringstream ws;
                    ws << pingdetailreadpage;
                    std::wstring wstrd = ws.str();
                    HWND page = GetDlgItem(hWnd, EDIT_pagereadNO); // 获取控件句柄
                    UpdateStaticText(page, wstrd.c_str());//更新
                }
                break;
            case IDM_daochu:
                SavePingDetailsToFile();
                break;
            case IDM_stop:
            {    //停止
                stop[ThreadID] = 1;

                HWND ping = GetDlgItem(hWnd, IDM_ping);
                EnableWindow(ping, TRUE); // 按钮变灰（不可点击）
                HWND checkBox = GetDlgItem(hWnd, IDM_checkBox);
                EnableWindow(checkBox, TRUE); // 按钮变灰（不可点击）
                HWND wenzi2 = GetDlgItem(hWnd, EDIT_wenzi2);
                EnableWindow(wenzi2, TRUE); // 文字变灰
                HWND stopbutton = GetDlgItem(hWnd, IDM_stop);
                EnableWindow(stopbutton, FALSE); // 按钮启用
                HWND slk = GetDlgItem(hWnd, EDIT_slk1);
                EnableWindow(slk, TRUE); // 启用编辑框

                break;
            }
            case ID_0s:
            case ID_5s:
            case ID_10s:
            case ID_30s:
            case ID_1min:
            case ID_5min:
            {
                // 主菜单句柄
                HMENU hMenu = GetMenu(hWnd);
                // 清除所有相关菜单项的勾选状态
                CheckMenuItem(hMenu, ID_0s, MF_BYCOMMAND | MF_UNCHECKED);
                CheckMenuItem(hMenu, ID_5s, MF_BYCOMMAND | MF_UNCHECKED);
                CheckMenuItem(hMenu, ID_10s, MF_BYCOMMAND | MF_UNCHECKED);
                CheckMenuItem(hMenu, ID_30s, MF_BYCOMMAND | MF_UNCHECKED);
                CheckMenuItem(hMenu, ID_1min, MF_BYCOMMAND | MF_UNCHECKED);
                CheckMenuItem(hMenu, ID_5min, MF_BYCOMMAND | MF_UNCHECKED);
                // 设置当前点击的菜单项为已勾选
                CheckMenuItem(hMenu, wParam, MF_BYCOMMAND | MF_CHECKED);
                // 修改sleeptime
                switch (wParam)
                {
                case ID_0s:     sleeptime = 0; break;
                case ID_5s:     sleeptime = 5; break;
                case ID_10s:    sleeptime = 10; break;
                case ID_30s:    sleeptime = 30; break;
                case ID_1min:   sleeptime = 60; break;
                case ID_5min:   sleeptime = 300; break;
                }
                break;
            }
            case IDM_jiance:
            {
                wchar_t buffer[256] = { 0 }; // 宽字符缓冲区
                HWND slk3 = GetDlgItem(hWnd, EDIT_slk3);//获取EDIT控件的句柄
                GetWindowTextW(slk3, buffer, 256); // 从窗口获取文本

                // 将宽字符转为 std::string
                int len = wcslen(buffer);
                std::string url(len * 2, 0);

                WideCharToMultiByte(CP_ACP, 0, buffer, -1, &url[0], (int)url.size(), nullptr, nullptr);
                url.resize(strlen(url.c_str())); // 去掉多余部分

                std::wstring output = GetDNSRecords(hWnd, url);//执行函数
                HWND hEdit = GetDlgItem(hWnd, EDIT_DNSoutput);

                {
                    // 获取静态文本控件的客户区矩形
                    RECT rc;
                    GetClientRect(hEdit, &rc);

                    // 获取设备上下文 (DC)
                    HDC hdc = GetDC(hEdit);
                    
                    // 使用白色画刷填充背景
                    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // 白色画刷
                    FillRect(hdc, &rc, hBrush); // 填充背景

                    // 释放资源
                    DeleteObject(hBrush);
                    ReleaseDC(hEdit, hdc);
                }

                SetWindowTextW(hEdit, output.c_str());
                break;
            }
            case ID_32772:
                system("start https://github.com/YXC-Lhy/Domain-Checker-Tool");
                break;
            case ID_32773:
                system("start https://space.bilibili.com/3493280161466993");
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CREATE:
        {
            
            hTabControl = CreateWindowEx(//创建标签页
                0,
                WC_TABCONTROL,
                L"",
                WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH | TCS_RAGGEDRIGHT | WS_EX_CONTROLPARENT,
                -1, 0, 588, 444, // 位置和大小 x,y,w,h
                hWnd,
                (HMENU)IDC_TAB_CONTROL,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );

            // 设置标签大小
            TabCtrl_SetItemSize(hTabControl, 195, 30);//w,h

            // 定义字体属性
            LOGFONT lf;
            ZeroMemory(&lf, sizeof(LOGFONT));
            lstrcpy(lf.lfFaceName, L"Microsoft YaHei"); // 设置字体名称
            //lf.lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72); // 设置字体大小（这里为12点）
            lf.lfWeight = FW_NORMAL; // 设置字体粗细
            // 创建字体
            HFONT hFont = CreateFontIndirect(&lf);

            HDC hdc = GetDC(NULL); // 获取整个屏幕的设备上下文
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            // 定义字体属性
            LOGFONT lf2;
            ZeroMemory(&lf2, sizeof(LOGFONT));
            lstrcpy(lf2.lfFaceName, L"Microsoft YaHei"); // 设置字体名称
            lf2.lfHeight = -MulDiv(22, GetDeviceCaps(hdc, LOGPIXELSY), 72); // 设置字体大小（这里为12点）
            lf2.lfWeight = FW_NORMAL; // 设置字体粗细
            // 创建字体
            HFONT bFont = CreateFontIndirect(&lf2);

            TCITEM tie;
            tie.mask = TCIF_TEXT;
            tie.pszText = (LPWSTR)L"Ping";
            TabCtrl_InsertItem(hTabControl, 0, &tie);
            tie.pszText = (LPWSTR)L"DNS";
            TabCtrl_InsertItem(hTabControl, 1, &tie);
            tie.pszText = (LPWSTR)L"关于";
            TabCtrl_InsertItem(hTabControl, 2, &tie);
            SendMessage(hTabControl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

            //第一页ping EDIT_Page1
            HWND wenzi1 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"域名/IP：\n\n响应IP：\n发包次数：\n接收次数：               接收率：     %\n丢包次数：               丢包率：     %\n延迟时间：平均：        ms   最长：        ms   最短：        ms\nTTL(Time To Live)：\n\n详细信息（每页2000行）：                                                第       /       页",
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                10, 55, 550, 350, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_Page1,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(wenzi1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND slk = CreateWindow(
                L"EDIT",
                L"www.bing.com",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                80, 55, 250, 30,
                hWnd, // 父窗口句柄
                (HMENU)EDIT_slk1,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(slk, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND pingbutton = CreateWindow(
                L"BUTTON",  // 窗口类名
                L"Ping", // 按钮上的文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // 样式
                340, 55, 60, 30, // 位置和大小
                hWnd, // 父窗口
                (HMENU)IDM_ping, // 控件ID
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(pingbutton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND stopbutton = CreateWindow(
                L"BUTTON",  // 窗口类名
                L"停止", // 按钮上的文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // 样式
                410, 55, 60, 30, // 位置和大小
                hWnd, // 父窗口
                (HMENU)IDM_stop, // 控件ID
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(stopbutton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            EnableWindow(stopbutton, FALSE); // 按钮变灰
            HWND checkBox = CreateWindowEx(
                0, L"BUTTON", L"",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,// 复选框控件
                480, 63, 13, 13,//x,y,w,h
                hWnd, (HMENU)IDM_checkBox, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(checkBox, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND wenzi2 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"持续测试", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                500, 58, 500, 30, // 位置和大小，x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_wenzi2,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(wenzi2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND IP = CreateWindow(
                L"STATIC",  // 窗口类名
                L"---.---.---.---", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                80, 97, 500, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_IP,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(IP, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND TTL = CreateWindow(
                L"STATIC",  // 窗口类名
                L"----", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                155, 202, 500, 30, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_TTL,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(TTL, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND ms1 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"----", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_RIGHT, // 样式(靠右！！！！！！）
                135, 182, 40, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_ms1,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(ms1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND msmax = CreateWindow(
                L"STATIC",  // 窗口类名
                L"----", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_RIGHT, // 样式(靠右！！！！！！）
                260, 182, 40, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_msmax,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(msmax, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND msmin = CreateWindow(
                L"STATIC",  // 窗口类名
                L"----", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_RIGHT, // 样式(靠右！！！！！！）
                385, 182, 40, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_msmin,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(msmin, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND ci = CreateWindow(
                L"STATIC",  // 窗口类名
                L"----", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                90, 119, 70, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_ci,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(ci, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND ci1 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"----", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                90, 139, 70, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_ci1,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(ci1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND ci2 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"----", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                90, 159, 70, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_ci2,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(ci2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND pc = CreateWindow(
                L"STATIC",  // 窗口类名
                L"---", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_RIGHT, // 样式(靠右！！！！！！
                222, 139, 30, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_percent,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(pc, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND pc2 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"---", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_RIGHT, // 样式(靠右！！！！！！
                222, 159, 30, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_percent2,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(pc2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND slk2 = CreateWindow(
                L"EDIT",
                L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE| ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                10, 275, 565, 155,
                hWnd, // 父窗口句柄
                (HMENU)EDIT_slk2,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(slk2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));\
            HWND pageNO = CreateWindow(
                L"STATIC",  // 窗口类名
                L"1", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_CENTER, // 样式(靠中！！！！！！
                507, 244, 30, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_pageNO,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(pageNO, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND pagereadNO = CreateWindow(
                L"STATIC",  // 窗口类名
                L"1", // 显示的文本
                WS_CHILD | WS_VISIBLE | SS_CENTER, // 样式(靠中！！！！！！
                465, 244, 30, 25, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_pagereadNO,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(pagereadNO, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND backpage = CreateWindow(
                L"BUTTON",  // 窗口类名
                L"上一页", // 按钮上的文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // 样式
                290, 242, 70, 27, // 位置和大小 (x, y, width, height)
                hWnd, // 父窗口
                (HMENU)IDM_backpage, // 控件ID
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(backpage, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND nextpage = CreateWindow(
                L"BUTTON",  // 窗口类名
                L"下一页", // 按钮上的文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // 样式
                365, 242, 70, 27, // 位置和大小 (x, y, width, height)
                hWnd, // 父窗口
                (HMENU)IDM_nextpage, // 控件ID
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(nextpage, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND daochu = CreateWindow(
                L"BUTTON",  // 窗口类名
                L"导出", // 按钮上的文本
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, // 样式
                210, 242, 70, 27, // 位置和大小 (x, y, width, height)
                hWnd, // 父窗口
                (HMENU)IDM_daochu, // 控件ID
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(daochu, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            
            //第二页DNS
            HWND wenzi3 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"域名：", // 显示的文本内容
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                10, 55, 500, 350, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_Page2,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(wenzi3, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND slk3 = CreateWindow(
                L"EDIT",
                L"www.bing.com",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                60, 55, 230, 30,
                hWnd, // 父窗口句柄
                (HMENU)EDIT_slk3,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(slk3, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND hCombo = CreateWindow(
                WC_COMBOBOX,
                L"",
                CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
                300, 55, 190, 80, 
                hWnd,
                (HMENU)COMBOBOX_1,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(hCombo, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"A (Address)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"AAAA (IPv6 Address)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"CNAME (Canonical Name)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"MX (Mail Exchange)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"NS (Name Server)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"PTR (Pointer)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"SOA (Start of Authority)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"TXT (Text Record)");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"SRV (Service Location)");
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);// 设置默认选中第一个选项
            HWND jiance = CreateWindow(
                L"BUTTON",
                L"检测",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                500, 55, 70, 30,
                hWnd, // 父窗口句柄
                (HMENU)IDM_jiance,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(jiance, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND DNSoutput = CreateWindow(
                L"EDIT",  // 窗口类名
                L"", // 显示的文本内容
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                10, 105, 565, 320, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_DNSoutput,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(DNSoutput, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            
            //第三页
            HWND wenzi5 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"域名检测工具",
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                10, 55, 550, 350, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_3,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(wenzi5, WM_SETFONT, (WPARAM)bFont, MAKELPARAM(TRUE, 0));
            HWND wenzi4 = CreateWindow(
                L"STATIC",  // 窗口类名
                L"\n\nDomain Checker Tool\n\n本工具由Lhy制作，语言为C++，UI绘制为Win32，相关功能调用了Windows的ping.exe和nslookup\n\n代码已开源在github，相关网站链接见菜单栏的“关于”\n\n在菜单栏可调节重复ping的间隔设置，次数暂不可调\n\n一部分代码由AI完成，如截屏和文本处理",
                WS_CHILD | WS_VISIBLE | SS_LEFT, // 样式
                10, 55, 550, 350, // 位置和大小x,y,w,h
                hWnd, // 父窗口句柄
                (HMENU)EDIT_Page3,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            SendMessage(wenzi4, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

            // 初始状态下只显示第一个标签页的内容
            ShowHideControls(1, 1, hWnd);
            ShowHideControls(2, 0, hWnd);
            ShowHideControls(3, 0, hWnd);

    }
    case WM_NOTIFY:
    {

        NMHDR* pnmh = (NMHDR*)lParam;
        int selectedIndex = TabCtrl_GetCurSel(hTabControl);
        /*if (pnmh->idFrom == IDC_TAB_CONTROL && pnmh->code == TCN_SELCHANGE)
        {

            ShowWindow(hPage1Container, selectedIndex == 0 ? SW_SHOW : SW_HIDE);
            ShowWindow(hPage2Container, selectedIndex == 1 ? SW_SHOW : SW_HIDE);
            ShowWindow(hPage3Container, selectedIndex == 2 ? SW_SHOW : SW_HIDE);
        }*/

        //处理切页显示
        if (selectedIndex == 0) {
            //ShowWindow(hStatic, SW_SHOW);
            ShowHideControls(1, 1, hWnd);
            ShowHideControls(2, 0, hWnd);
            ShowHideControls(3, 0, hWnd);
        }
        else if (selectedIndex == 1) {
            //ShowWindow(hStatic, SW_HIDE);
            ShowHideControls(1, 0, hWnd);
            ShowHideControls(2, 1, hWnd);
            ShowHideControls(3, 0, hWnd);
        }
        else {
            //ShowWindow(hStatic, SW_HIDE);
            ShowHideControls(1, 0, hWnd);
            ShowHideControls(2, 0, hWnd);
            ShowHideControls(3, 1, hWnd);
        }

        break;
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
