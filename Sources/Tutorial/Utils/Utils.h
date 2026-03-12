#pragma once

#ifndef __UTILS_H__
#define __UTILS_H__

#define myAssert( cond, message )   { if (!(cond)) ShowErrorMessageBox( message); }

void ShowErrorMessageBox( LPCWSTR message );

void ConvertStringToWideString( const char *_szString, WCHAR *_szOutWideString );
void ConvertWdieStringToString( const WCHAR *_szWideString, char *_szOutString );

#define V_RETURN(x)                 { hr = (x); if( FAILED(hr) ) { return hr; } }

#define SAFE_DELETE(x)              { if (x) { delete (x); (x) = NULL; } }
#define SAFE_RELEASE(x)             { if (x) { (x)->Release(); (x) = NULL; } }
#define SAFE_FREE(x)                { if (x) { free(x); (x) = NULL; } }

#define D3D_DEBUG_SET_NAME(x, name) { (x)->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof(name)-1, name ); }

#endif // __UTILS_H__
