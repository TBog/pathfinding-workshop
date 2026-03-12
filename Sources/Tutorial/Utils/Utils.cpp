#include "pch.h"

#include "Utils.h"

//-------------------------------------------------------------------

void ShowErrorMessageBox( LPCWSTR message )
{
    MessageBox( NULL, message, L"Error", MB_OK | MB_ICONERROR );
}

//-------------------------------------------------------------------

void ConvertStringToWideString( const char *_szString, WCHAR *_szOutWideString )
{
    size_t length = strlen( _szString ) + 1;

    mbstowcs_s( &length, _szOutWideString, length, _szString, length );
}

//-------------------------------------------------------------------

void ConvertWdieStringToString(const WCHAR *_szWideString, char *_szOutString)
{
    size_t length = wcslen( _szWideString ) + 1;

    wcstombs_s( &length, _szOutString, length, _szWideString, length );
}

//-------------------------------------------------------------------
