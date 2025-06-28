#pragma once

#include <windows.h>
#include <string>

std::wstring GetTime()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    // ∏Ò Ω: [6-25 15:10:30]
    wchar_t buffer[64];
    swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]),
        L"[%d-%02d %02d:%02d:%02d]",
        st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    return std::wstring(buffer);
}