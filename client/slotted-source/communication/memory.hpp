#pragma once
#include <unordered_map>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

inline HWND bWindowHandle;
inline uintptr_t Base;

typedef struct _BA
{
    INT32 Security;
    INT32 ProcessID;
    ULONGLONG* Address;
} BA , * PBA;
typedef struct _ReadWrite
{
    INT32 Security;
    INT32 ProcessID;
    ULONGLONG Address;
    ULONGLONG Buffer;
    ULONGLONG Size;
    BOOLEAN Write;
    BOOLEAN EAC;
} RW , * PRW;

typedef struct _DTB
{
    INT32 Security;
    INT32 ProcessID;
    bool* Operation;
} DTB , * DTBL;
typedef struct MOUSE_INIT
{
    INT32 Security;
    INT32 Reserved;
    ULONGLONG DeviceHandle;
    ULONGLONG Callback;
    BOOLEAN Initialized;
    BOOLEAN EAC;
} MOUSE_INIT , * MOUSE_INIT_;

typedef struct MOUSE_MOVE
{
    INT32 X;
    INT32 Y;
    UINT16 ButtonFlags;
    BOOLEAN EAC;
    ULONGLONG Reserved1;
    ULONGLONG Reserved2;
} MOUSE_MOVE , * MOUSE_MOVE_;

namespace fortnite {
    namespace communcations
    {
        inline HANDLE driver_handle = INVALID_HANDLE_VALUE;
        inline int process_id = 0;
        inline uintptr_t base_address;

        inline bool find_driver( )
        {
            driver_handle = CreateFileW( L"\\\\.\\WinKernelInterface" , GENERIC_READ | GENERIC_WRITE , 0 , nullptr , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , nullptr );
            if ( driver_handle == INVALID_HANDLE_VALUE )
            {
                return false;
            }
            return true;
        }

        inline uintptr_t get_base( )
        {
            uintptr_t image_address = 0;
            _BA Arguments {};

            Arguments.Security = 0x1E2E3F;
            Arguments.ProcessID = process_id;
            Arguments.Address = ( ULONGLONG* ) &image_address;

            DeviceIoControl( driver_handle , 0x1889 , &Arguments , sizeof( Arguments ) , nullptr , 0 , nullptr , nullptr );
            base_address = image_address;
            Base = image_address;
            return image_address;
        }

        inline INT32 get_process_id( LPCTSTR process_name )
        {
            PROCESSENTRY32 pt;
            HANDLE hsnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS , 0 );
            pt.dwSize = sizeof( PROCESSENTRY32 );
            if ( Process32First( hsnap , &pt ) ) {
                do {
                    if ( !lstrcmpi( pt.szExeFile , process_name ) )
                    {
                        CloseHandle( hsnap );
                        process_id = pt.th32ProcessID;
                        return pt.th32ProcessID;
                    }
                } while ( Process32Next( hsnap , &pt ) );
            }

            CloseHandle( hsnap );
            return process_id;
        }

        inline bool read_memory( PVOID address , PVOID buffer , DWORD size )
        {
            _ReadWrite Arguments = { 0 };
            Arguments.Security = 0x1E2E3F;
            Arguments.Address = ( ULONGLONG ) address;
            Arguments.Buffer = ( ULONGLONG ) buffer;
            Arguments.Size = size;
            Arguments.ProcessID = process_id;
            Arguments.Write = FALSE;
            Arguments.EAC = TRUE;

            return DeviceIoControl( driver_handle , 0x1999 , &Arguments , sizeof( Arguments ) , nullptr , 0 , nullptr , nullptr );
        }

        inline bool write_memory( PVOID address , PVOID buffer , DWORD size )
        {
            _ReadWrite Arguments = { 0 };
            Arguments.Security = 0x1E2E3F;
            Arguments.Address = ( ULONGLONG ) address;
            Arguments.Buffer = ( ULONGLONG ) buffer;
            Arguments.Size = size;
            Arguments.ProcessID = process_id;
            Arguments.Write = TRUE;
            Arguments.EAC = TRUE;

            return DeviceIoControl( driver_handle , 0x1999 , &Arguments , sizeof( Arguments ) , nullptr , 0 , nullptr , nullptr );
        }

        inline bool get_cr3( )
        {
            bool Ret = false;
            _DTB Arguments = { 0 };
            Arguments.Security = 0x1E2E3F;
            Arguments.ProcessID = process_id;
            Arguments.Operation = ( bool* ) &Ret;

            return DeviceIoControl( driver_handle , 0x1299 , &Arguments , sizeof( Arguments ) , nullptr , 0 , nullptr , nullptr );
        }

        template<typename T>
        inline T read( uintptr_t address )
        {
            T buffer {};
            read_memory( ( PVOID ) address , &buffer , sizeof( T ) );
            return buffer;
        }

        template<typename T>
        inline T write( uintptr_t address , const T& data )
        {
            T buffer {};
            write_memory( reinterpret_cast< PVOID >( address ) , const_cast< T* >( &data ) , sizeof( T ) );
            return buffer;
        }
    }
}
