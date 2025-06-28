#pragma once


#include <Windows.h>
#include <commdlg.h>  // 文件对话框
#include <stdio.h>


// 更新静态文本控件的文本并确保背景被正确重绘函数
void UpdateStaticText(HWND hStatic, LPCWSTR newText)
{
    if (!hStatic)
        return;

    // 获取静态文本控件的客户区矩形
    RECT rc;
    GetClientRect(hStatic, &rc);

    // 获取设备上下文 (DC)
    HDC hdc = GetDC(hStatic);
    if (hdc == NULL)
        return;

    // 使用白色画刷填充背景
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // 白色画刷
    FillRect(hdc, &rc, hBrush); // 填充背景

    // 释放资源
    DeleteObject(hBrush);
    ReleaseDC(hStatic, hdc);

    // 设置新的文本
    SendMessage(hStatic, WM_SETTEXT, 0, (LPARAM)newText);

    // 强制刷新控件以确保立即显示更改
    RedrawWindow(hStatic, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}


// 显示或隐藏控件函数(页数，0隐藏,1显示，父窗口句柄)
void ShowHideControls(int k, bool bShow, HWND hWndParent)
{

    // 遍历所有定义的控件ID组
    for (int id = (k * 1000); id <= (k * 1000 + 999); ++id)
    {
        HWND hWndCtrl = GetDlgItem(hWndParent, id);
        if (hWndCtrl != NULL)
        {
            ShowWindow(hWndCtrl, bShow ? SW_SHOW : SW_HIDE);
        }
    }
}


BOOL CaptureWindowToFile(HWND hwnd)//此截屏函数来自deepseek
{
    if (!hwnd || !IsWindow(hwnd))
    {
        MessageBox(NULL, L"无效的窗口句柄", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 获取窗口尺寸
    RECT rect;
    GetWindowRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // 获取窗口 DC
    HDC hdcWindow = GetDC(hwnd);
    if (!hdcWindow)
    {
        MessageBox(NULL, L"无法获取窗口DC", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 创建兼容的内存 DC
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    if (!hdcMem)
    {
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"无法创建内存DC", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 创建兼容位图（24位色深）
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcWindow, width, height);
    if (!hbmMem)
    {
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"无法创建位图", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 将位图选入内存 DC
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // 复制窗口内容到内存 DC
    if (!PrintWindow(hwnd, hdcMem, 0))
    {
        // 如果 PrintWindow 失败，改用 BitBlt
        if (!BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY))
        {
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            ReleaseDC(hwnd, hdcWindow);
            MessageBox(NULL, L"无法捕获窗口内容", L"错误", MB_OK | MB_ICONERROR);
            return FALSE;
        }
    }

    // 准备 BMP 文件头和信息头
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;  // 24位色深（RGB）
    bi.biCompression = BI_RGB;

    // 计算每行像素的字节数（BMP 要求每行字节数是 4 的倍数）
    DWORD dwBmpSize = ((width * 3 + 3) & ~3) * height;

    // 分配内存存储位图数据
    BYTE* lpBits = new BYTE[dwBmpSize];
    if (!lpBits)
    {
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"内存不足", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 获取位图数据
    if (!GetDIBits(hdcMem, hbmMem, 0, height, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
    {
        delete[] lpBits;
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"无法获取位图数据", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 使用 Win32 文件对话框选择保存位置
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = L"";

    // 获取时间
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // 构造默认文件名
    wchar_t defaultFileName[256];
    swprintf(defaultFileName, 256, L"screenshot_%04d-%02d-%02d.bmp",st.wYear, st.wMonth, st.wDay);
    wcscpy_s(szFile, MAX_PATH, defaultFileName);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"24位位图(*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"bmp";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    BOOL bResult = FALSE;
    if (GetSaveFileName(&ofn))
    {
        // 准备 BMP 文件头
        BITMAPFILEHEADER bf = { 0 };
        bf.bfType = 0x4D42;  // "BM"
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        // 写入文件
        FILE* pFile;
        _wfopen_s(&pFile, szFile, L"wb");
        if (pFile)
        {
            fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, pFile);
            fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, pFile);
            fwrite(lpBits, dwBmpSize, 1, pFile);
            fclose(pFile);
            bResult = TRUE;
        }
        else
        {
            MessageBox(NULL, L"无法保存文件", L"错误", MB_OK | MB_ICONERROR);
        }
    }

    // 释放资源
    delete[] lpBits;
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);

    return bResult;
}

/*
#include <Windows.h>
#include <dwmapi.h>      // DWM 相关 API
#include <commdlg.h>     // 文件对话框
#include <stdio.h>       // 文件操作
#pragma comment(lib, "Dwmapi.lib")  // 链接 DWM 库

BOOL CaptureWindowToFile(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd))
    {
        MessageBox(NULL, L"无效的窗口句柄", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 获取窗口尺寸（包括阴影等 DWM 扩展区域）
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // 获取窗口 DC
    HDC hdcWindow = GetDC(hwnd);
    if (!hdcWindow)
    {
        MessageBox(NULL, L"无法获取窗口DC", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 创建内存 DC 和位图
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcWindow, width, height);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // 尝试用 DWM 方式捕获完整窗口（包括透明/阴影效果）
    BOOL bUseDWM = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &bUseDWM, sizeof(BOOL))) && !bUseDWM)
    {
        // PW_RENDERFULLCONTENT 捕获 DWM 合成的内容（Win8+）
        PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);
    }
    else
    {
        // 回退到 BitBlt（可能没有现代样式）
        BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
    }

    // 准备 BMP 文件头和信息头
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;  // BMP 是倒向存储的，高度为正表示从上到下
    bi.biPlanes = 1;
    bi.biBitCount = 24;    // 24 位色（RGB）
    bi.biCompression = BI_RGB;

    // 计算每行像素的字节数（BMP 要求每行按 4 字节对齐）
    DWORD dwStride = (width * 3 + 3) & ~3;  // (width * 3) 向上取整到 4 的倍数
    DWORD dwBmpSize = dwStride * height;

    // 分配内存存储位图数据
    BYTE* lpBits = new BYTE[dwBmpSize];
    if (!lpBits)
    {
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"内存不足", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 获取位图数据
    if (!GetDIBits(hdcMem, hbmMem, 0, height, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
    {
        delete[] lpBits;
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"无法获取位图数据", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 使用文件对话框选择保存位置
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = L"screenshot.bmp";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"bmp";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    BOOL bResult = FALSE;
    if (GetSaveFileName(&ofn))
    {
        // 准备 BMP 文件头
        BITMAPFILEHEADER bf = { 0 };
        bf.bfType = 0x4D42;  // "BM"
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        // 写入文件
        FILE* pFile;
        _wfopen_s(&pFile, szFile, L"wb");
        if (pFile)
        {
            fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, pFile);
            fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, pFile);
            fwrite(lpBits, dwBmpSize, 1, pFile);
            fclose(pFile);
            bResult = TRUE;
        }
        else
        {
            MessageBox(NULL, L"无法保存文件", L"错误", MB_OK | MB_ICONERROR);
        }
    }

    // 释放资源
    delete[] lpBits;
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);

    return bResult;
}
*/

#include "define.h"
#include "ping.h"
#include <fstream>
#include <string>
#include <sstream>

void SavePingDetailsToFile()
{
    wchar_t szFileName[MAX_PATH] = L"";
    wchar_t szFilter[] = L"文本文档 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0";

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWndMain; // 主窗口句柄
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"txt";

    // 获取EDIT_slk1文本
    wchar_t slk1Text[256];
    HWND edit = GetDlgItem(hWndMain, EDIT_slk1);
    GetWindowTextW(edit, slk1Text, 256);

    // 获取时间
    SYSTEMTIME st;
    GetLocalTime(&st);

    // 构造默认文件名
    wchar_t defaultFileName[256];
    swprintf(defaultFileName, 256, L"detail_%s_%04d-%02d-%02d.txt",
        slk1Text, st.wYear, st.wMonth, st.wDay);

    wcscpy_s(szFileName, MAX_PATH, defaultFileName);

    // 显示“另存为”对话框
    if (GetSaveFileName(&ofn) and pingi2 != 0)
    {
        std::wofstream outFile(ofn.lpstrFile);
        if (outFile.is_open())
        {
            outFile << "域名/IP：" << slk1Text << std::endl;
            float a = pingi4 * 100 / pingi2;
            a = round(a * 10) / 10;
            outFile << "发包次数：" << pingi2 << std::endl;
            outFile << "接收次数：" << pingi4 << "  接收率：" << a << "%" << std::endl;
            outFile << "丢包次数：" << pingi2 - pingi4 << "  丢包率：" << 100 - a << "%" << std::endl;
            if (pingi4 != 0) outFile << "延迟时间：平均：" << pingi / pingi4 << " ms 最长：" << pingmax << " ms 最短：" << pingmin << " ms" << std::endl;
            HWND TTL = GetDlgItem(hWndMain, EDIT_TTL);
            wchar_t ttl[256];
            GetWindowText(TTL, ttl, 256);
            outFile << "TTL(Time To Live)：" << ttl << std::endl << std::endl << "详细信息：" << std::endl;
            for (int i = 1; i <= pingdetailpage; i++)
            {
                outFile << pingdetail[i].str(); // 写入每页的内容
                //if (i != pingdetailpage - 1)
                    //outFile << std::endl; // 每个数组之间加换行（可选）
            }
            //MessageBox(hWndMain, L"打开文件进行写入完成", L"正确", MB_OK | MB_ICONERROR);
            outFile.close();
        }
        else
        {
            MessageBox(hWndMain, L"无法打开文件进行写入", L"错误", MB_OK | MB_ICONERROR);
        }
    }
}



