/*
 * Ntdll Unix interface
 *
 * Copyright (C) 2020 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __NTDLL_UNIXLIB_H
#define __NTDLL_UNIXLIB_H

#include "wine/server.h"
#include "wine/debug.h"

struct msghdr;
struct _DISPATCHER_CONTEXT;

/* increment this when you change the function table */
#define NTDLL_UNIXLIB_VERSION 97

struct unix_funcs
{
    /* Nt* functions */
    TEB *         (WINAPI *NtCurrentTeb)(void);

    /* other Win32 API functions */
    NTSTATUS      (WINAPI *DbgUiIssueRemoteBreakin)( HANDLE process );
    LONGLONG      (WINAPI *RtlGetSystemTimePrecise)(void);
    NTSTATUS      (WINAPI *RtlWaitOnAddress)( const void *addr, const void *cmp, SIZE_T size,
                                              const LARGE_INTEGER *timeout );
    void          (WINAPI *RtlWakeAddressAll)( const void *addr );
    void          (WINAPI *RtlWakeAddressSingle)( const void *addr );

    /* fast locks */
    NTSTATUS      (CDECL *fast_RtlpWaitForCriticalSection)( RTL_CRITICAL_SECTION *crit, int timeout );
    NTSTATUS      (CDECL *fast_RtlpUnWaitCriticalSection)( RTL_CRITICAL_SECTION *crit );
    NTSTATUS      (CDECL *fast_RtlDeleteCriticalSection)( RTL_CRITICAL_SECTION *crit );
    NTSTATUS      (CDECL *fast_RtlTryAcquireSRWLockExclusive)( RTL_SRWLOCK *lock );
    NTSTATUS      (CDECL *fast_RtlAcquireSRWLockExclusive)( RTL_SRWLOCK *lock );
    NTSTATUS      (CDECL *fast_RtlTryAcquireSRWLockShared)( RTL_SRWLOCK *lock );
    NTSTATUS      (CDECL *fast_RtlAcquireSRWLockShared)( RTL_SRWLOCK *lock );
    NTSTATUS      (CDECL *fast_RtlReleaseSRWLockExclusive)( RTL_SRWLOCK *lock );
    NTSTATUS      (CDECL *fast_RtlReleaseSRWLockShared)( RTL_SRWLOCK *lock );
    NTSTATUS      (CDECL *fast_RtlWakeConditionVariable)( RTL_CONDITION_VARIABLE *variable, int count );
    NTSTATUS      (CDECL *fast_wait_cv)( RTL_CONDITION_VARIABLE *variable, const void *value,
                                         const LARGE_INTEGER *timeout );

    /* math functions */
    double        (CDECL *atan)( double d );
    double        (CDECL *ceil)( double d );
    double        (CDECL *cos)( double d );
    double        (CDECL *fabs)( double d );
    double        (CDECL *floor)( double d );
    double        (CDECL *log)( double d );
    double        (CDECL *pow)( double x, double y );
    double        (CDECL *sin)( double d );
    double        (CDECL *sqrt)( double d );
    double        (CDECL *tan)( double d );

    /* environment functions */
    NTSTATUS      (CDECL *get_initial_environment)( WCHAR **wargv[], WCHAR *env, SIZE_T *size );
    NTSTATUS      (CDECL *get_startup_info)( startup_info_t *info, SIZE_T *total_size, SIZE_T *info_size );
    NTSTATUS      (CDECL *get_dynamic_environment)( WCHAR *env, SIZE_T *size );
    void          (CDECL *get_initial_console)( HANDLE *handle, HANDLE *std_in,
                                                HANDLE *std_out, HANDLE *std_err );
    void          (CDECL *get_initial_directory)( UNICODE_STRING *dir );
    USHORT *      (CDECL *get_unix_codepage_data)(void);
    void          (CDECL *get_locales)( WCHAR *sys, WCHAR *user );

    /* virtual memory functions */
    NTSTATUS      (CDECL *virtual_map_section)( HANDLE handle, PVOID *addr_ptr, unsigned short zero_bits_64, SIZE_T commit_size,
                                                const LARGE_INTEGER *offset_ptr, SIZE_T *size_ptr, ULONG alloc_type,
                                                ULONG protect, pe_image_info_t *image_info );
    ssize_t       (CDECL *virtual_locked_recvmsg)( int fd, struct msghdr *hdr, int flags );
    void          (CDECL *virtual_release_address_space)(void);

    /* thread/process functions */
    NTSTATUS      (CDECL *exec_process)( UNICODE_STRING *path, UNICODE_STRING *cmdline, NTSTATUS status );

    /* server functions */
    void          (CDECL *server_init_process_done)( void *relay );

    /* file functions */
    void          (CDECL *set_show_dot_files)( BOOL enable );

    /* loader functions */
    NTSTATUS      (CDECL *load_so_dll)( UNICODE_STRING *nt_name, void **module );
    NTSTATUS      (CDECL *load_builtin_dll)( const WCHAR *name, void **module,
                                             pe_image_info_t *image_info );
    NTSTATUS      (CDECL *unload_builtin_dll)( void *module );
    void          (CDECL *init_builtin_dll)( void *module );
    NTSTATUS      (CDECL *unwind_builtin_dll)( ULONG type, struct _DISPATCHER_CONTEXT *dispatch,
                                               CONTEXT *context );

    /* debugging functions */
    unsigned char (CDECL *dbg_get_channel_flags)( struct __wine_debug_channel *channel );
    const char *  (CDECL *dbg_strdup)( const char *str );
    int           (CDECL *dbg_output)( const char *str );
    int           (CDECL *dbg_header)( enum __wine_debug_class cls, struct __wine_debug_channel *channel,
                                       const char *function );
};

#endif /* __NTDLL_UNIXLIB_H */
