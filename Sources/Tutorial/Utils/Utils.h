#pragma once

#ifndef __UTILS_H__
#define __UTILS_H__

/// @file Utils.h
/// @brief Common utility macros and helper functions used throughout the engine.

/// @brief Displays a message box if @p cond evaluates to @c false.
///
/// Use this in place of a standard @c assert when a human-readable error
/// message is required.  The message box is shown synchronously; execution
/// continues after the dialog is dismissed.
///
/// @param cond    Boolean condition that must be @c true.
/// @param message Wide-character string shown in the message box on failure.
#define myAssert( cond, message )   { if (!(cond)) ShowErrorMessageBox( message); }

/// @brief Shows a modal error message box with the supplied wide-character string.
/// @param message The text to display.
void ShowErrorMessageBox( LPCWSTR message );

/// @brief Converts a narrow (multi-byte) string to a wide-character string.
/// @param _szString       Input narrow string (null-terminated).
/// @param _szOutWideString Output buffer for the wide-character string.
void ConvertStringToWideString( const char *_szString, WCHAR *_szOutWideString );

/// @brief Converts a wide-character string to a narrow (multi-byte) string.
/// @param _szWideString  Input wide-character string (null-terminated).
/// @param _szOutString   Output buffer for the narrow string.
void ConvertWdieStringToString( const WCHAR *_szWideString, char *_szOutString );

/// @brief Evaluates an HRESULT expression and returns it on failure.
///
/// Assigns the result of @p x to a local @c hr variable.  If @c FAILED(hr),
/// returns @c hr immediately.  Requires a local @c HRESULT variable named
/// @c hr to be in scope.
#define V_RETURN(x)                 { hr = (x); if( FAILED(hr) ) { return hr; } }

/// @brief Safely deletes a pointer and sets it to @c nullptr.
/// @param x Pointer to delete; no-op if already @c nullptr.
#define SAFE_DELETE(x)              { if (x) { delete (x); (x) = NULL; } }

/// @brief Calls @c Release() on a pointer and sets it to @c nullptr.
///
/// Suitable for @ref RefCounted objects, Direct3D COM objects, or any type
/// that exposes a @c Release() method.
/// @param x Pointer whose @c Release() to call; no-op if already @c nullptr.
#define SAFE_RELEASE(x)             { if (x) { (x)->Release(); (x) = NULL; } }

/// @brief Calls @c free() on a pointer and sets it to @c nullptr.
/// @param x Pointer to free; no-op if already @c nullptr.
#define SAFE_FREE(x)                { if (x) { free(x); (x) = NULL; } }

/// @brief Attaches a debug name string to a Direct3D 11 device object.
///
/// Uses @c SetPrivateData with @c WKPDID_D3DDebugObjectName so that the name
/// appears in graphics debuggers (e.g. PIX, RenderDoc).
/// @param x    Pointer to the D3D11 object.
/// @param name A string literal used as the debug name.
#define D3D_DEBUG_SET_NAME(x, name) { (x)->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof(name)-1, name ); }

#endif // __UTILS_H__
