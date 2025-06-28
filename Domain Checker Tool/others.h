#pragma once


#include <Windows.h>
#include <commdlg.h>  // �ļ��Ի���
#include <stdio.h>


// ���¾�̬�ı��ؼ����ı���ȷ����������ȷ�ػ溯��
void UpdateStaticText(HWND hStatic, LPCWSTR newText)
{
    if (!hStatic)
        return;

    // ��ȡ��̬�ı��ؼ��Ŀͻ�������
    RECT rc;
    GetClientRect(hStatic, &rc);

    // ��ȡ�豸������ (DC)
    HDC hdc = GetDC(hStatic);
    if (hdc == NULL)
        return;

    // ʹ�ð�ɫ��ˢ��䱳��
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // ��ɫ��ˢ
    FillRect(hdc, &rc, hBrush); // ��䱳��

    // �ͷ���Դ
    DeleteObject(hBrush);
    ReleaseDC(hStatic, hdc);

    // �����µ��ı�
    SendMessage(hStatic, WM_SETTEXT, 0, (LPARAM)newText);

    // ǿ��ˢ�¿ؼ���ȷ��������ʾ����
    RedrawWindow(hStatic, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}


// ��ʾ�����ؿؼ�����(ҳ����0����,1��ʾ�������ھ��)
void ShowHideControls(int k, bool bShow, HWND hWndParent)
{

    // �������ж���Ŀؼ�ID��
    for (int id = (k * 1000); id <= (k * 1000 + 999); ++id)
    {
        HWND hWndCtrl = GetDlgItem(hWndParent, id);
        if (hWndCtrl != NULL)
        {
            ShowWindow(hWndCtrl, bShow ? SW_SHOW : SW_HIDE);
        }
    }
}


BOOL CaptureWindowToFile(HWND hwnd)//�˽�����������deepseek
{
    if (!hwnd || !IsWindow(hwnd))
    {
        MessageBox(NULL, L"��Ч�Ĵ��ھ��", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ��ȡ���ڳߴ�
    RECT rect;
    GetWindowRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // ��ȡ���� DC
    HDC hdcWindow = GetDC(hwnd);
    if (!hdcWindow)
    {
        MessageBox(NULL, L"�޷���ȡ����DC", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // �������ݵ��ڴ� DC
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    if (!hdcMem)
    {
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"�޷������ڴ�DC", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ��������λͼ��24λɫ�
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcWindow, width, height);
    if (!hbmMem)
    {
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"�޷�����λͼ", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ��λͼѡ���ڴ� DC
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // ���ƴ������ݵ��ڴ� DC
    if (!PrintWindow(hwnd, hdcMem, 0))
    {
        // ��� PrintWindow ʧ�ܣ����� BitBlt
        if (!BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY))
        {
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            ReleaseDC(hwnd, hdcWindow);
            MessageBox(NULL, L"�޷����񴰿�����", L"����", MB_OK | MB_ICONERROR);
            return FALSE;
        }
    }

    // ׼�� BMP �ļ�ͷ����Ϣͷ
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;  // 24λɫ�RGB��
    bi.biCompression = BI_RGB;

    // ����ÿ�����ص��ֽ�����BMP Ҫ��ÿ���ֽ����� 4 �ı�����
    DWORD dwBmpSize = ((width * 3 + 3) & ~3) * height;

    // �����ڴ�洢λͼ����
    BYTE* lpBits = new BYTE[dwBmpSize];
    if (!lpBits)
    {
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"�ڴ治��", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ��ȡλͼ����
    if (!GetDIBits(hdcMem, hbmMem, 0, height, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
    {
        delete[] lpBits;
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"�޷���ȡλͼ����", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ʹ�� Win32 �ļ��Ի���ѡ�񱣴�λ��
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = L"";

    // ��ȡʱ��
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // ����Ĭ���ļ���
    wchar_t defaultFileName[256];
    swprintf(defaultFileName, 256, L"screenshot_%04d-%02d-%02d.bmp",st.wYear, st.wMonth, st.wDay);
    wcscpy_s(szFile, MAX_PATH, defaultFileName);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"24λλͼ(*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"bmp";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    BOOL bResult = FALSE;
    if (GetSaveFileName(&ofn))
    {
        // ׼�� BMP �ļ�ͷ
        BITMAPFILEHEADER bf = { 0 };
        bf.bfType = 0x4D42;  // "BM"
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        // д���ļ�
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
            MessageBox(NULL, L"�޷������ļ�", L"����", MB_OK | MB_ICONERROR);
        }
    }

    // �ͷ���Դ
    delete[] lpBits;
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);

    return bResult;
}

/*
#include <Windows.h>
#include <dwmapi.h>      // DWM ��� API
#include <commdlg.h>     // �ļ��Ի���
#include <stdio.h>       // �ļ�����
#pragma comment(lib, "Dwmapi.lib")  // ���� DWM ��

BOOL CaptureWindowToFile(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd))
    {
        MessageBox(NULL, L"��Ч�Ĵ��ھ��", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ��ȡ���ڳߴ磨������Ӱ�� DWM ��չ����
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // ��ȡ���� DC
    HDC hdcWindow = GetDC(hwnd);
    if (!hdcWindow)
    {
        MessageBox(NULL, L"�޷���ȡ����DC", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // �����ڴ� DC ��λͼ
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcWindow, width, height);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // ������ DWM ��ʽ�����������ڣ�����͸��/��ӰЧ����
    BOOL bUseDWM = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &bUseDWM, sizeof(BOOL))) && !bUseDWM)
    {
        // PW_RENDERFULLCONTENT ���� DWM �ϳɵ����ݣ�Win8+��
        PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);
    }
    else
    {
        // ���˵� BitBlt������û���ִ���ʽ��
        BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);
    }

    // ׼�� BMP �ļ�ͷ����Ϣͷ
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;  // BMP �ǵ���洢�ģ��߶�Ϊ����ʾ���ϵ���
    bi.biPlanes = 1;
    bi.biBitCount = 24;    // 24 λɫ��RGB��
    bi.biCompression = BI_RGB;

    // ����ÿ�����ص��ֽ�����BMP Ҫ��ÿ�а� 4 �ֽڶ��룩
    DWORD dwStride = (width * 3 + 3) & ~3;  // (width * 3) ����ȡ���� 4 �ı���
    DWORD dwBmpSize = dwStride * height;

    // �����ڴ�洢λͼ����
    BYTE* lpBits = new BYTE[dwBmpSize];
    if (!lpBits)
    {
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"�ڴ治��", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ��ȡλͼ����
    if (!GetDIBits(hdcMem, hbmMem, 0, height, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
    {
        delete[] lpBits;
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
        MessageBox(NULL, L"�޷���ȡλͼ����", L"����", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // ʹ���ļ��Ի���ѡ�񱣴�λ��
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
        // ׼�� BMP �ļ�ͷ
        BITMAPFILEHEADER bf = { 0 };
        bf.bfType = 0x4D42;  // "BM"
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
        bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        // д���ļ�
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
            MessageBox(NULL, L"�޷������ļ�", L"����", MB_OK | MB_ICONERROR);
        }
    }

    // �ͷ���Դ
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
    wchar_t szFilter[] = L"�ı��ĵ� (*.txt)\0*.txt\0�����ļ� (*.*)\0*.*\0";

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWndMain; // �����ھ��
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"txt";

    // ��ȡEDIT_slk1�ı�
    wchar_t slk1Text[256];
    HWND edit = GetDlgItem(hWndMain, EDIT_slk1);
    GetWindowTextW(edit, slk1Text, 256);

    // ��ȡʱ��
    SYSTEMTIME st;
    GetLocalTime(&st);

    // ����Ĭ���ļ���
    wchar_t defaultFileName[256];
    swprintf(defaultFileName, 256, L"detail_%s_%04d-%02d-%02d.txt",
        slk1Text, st.wYear, st.wMonth, st.wDay);

    wcscpy_s(szFileName, MAX_PATH, defaultFileName);

    // ��ʾ�����Ϊ���Ի���
    if (GetSaveFileName(&ofn) and pingi2 != 0)
    {
        std::wofstream outFile(ofn.lpstrFile);
        if (outFile.is_open())
        {
            outFile << "����/IP��" << slk1Text << std::endl;
            float a = pingi4 * 100 / pingi2;
            a = round(a * 10) / 10;
            outFile << "����������" << pingi2 << std::endl;
            outFile << "���մ�����" << pingi4 << "  �����ʣ�" << a << "%" << std::endl;
            outFile << "����������" << pingi2 - pingi4 << "  �����ʣ�" << 100 - a << "%" << std::endl;
            if (pingi4 != 0) outFile << "�ӳ�ʱ�䣺ƽ����" << pingi / pingi4 << " ms ���" << pingmax << " ms ��̣�" << pingmin << " ms" << std::endl;
            HWND TTL = GetDlgItem(hWndMain, EDIT_TTL);
            wchar_t ttl[256];
            GetWindowText(TTL, ttl, 256);
            outFile << "TTL(Time To Live)��" << ttl << std::endl << std::endl << "��ϸ��Ϣ��" << std::endl;
            for (int i = 1; i <= pingdetailpage; i++)
            {
                outFile << pingdetail[i].str(); // д��ÿҳ������
                //if (i != pingdetailpage - 1)
                    //outFile << std::endl; // ÿ������֮��ӻ��У���ѡ��
            }
            //MessageBox(hWndMain, L"���ļ�����д�����", L"��ȷ", MB_OK | MB_ICONERROR);
            outFile.close();
        }
        else
        {
            MessageBox(hWndMain, L"�޷����ļ�����д��", L"����", MB_OK | MB_ICONERROR);
        }
    }
}



