/*
 *	basic interfaces
 *
 *	Copyright 1997	Marcus Meissner
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "winerror.h"
#include "ldt.h"
#include "heap.h"
#include "wine/winbase16.h"
#include "wine/obj_base.h"
#include "objbase.h"
#include "local.h"
#include "module.h"
#include "debug.h"

#include "ifs.h"

/* --- IUnknown implementation */

typedef struct
{
    /* IUnknown fields */
    ICOM_VTABLE(IUnknown)* lpvtbl;
    DWORD                  ref;
} IUnknownImpl;

/******************************************************************************
 *		IUnknown_AddRef	[VTABLE:IUNKNOWN.1]
 */
static ULONG WINAPI IUnknown_fnAddRef(LPUNKNOWN iface) { 
	ICOM_THIS(IUnknownImpl,iface);
	TRACE(relay,"(%p)->AddRef()\n",This);
	return ++(This->ref);
}

/******************************************************************************
 * IUnknown_Release [VTABLE:IUNKNOWN.2]
 */
static ULONG WINAPI IUnknown_fnRelease(LPUNKNOWN iface) {
	ICOM_THIS(IUnknownImpl,iface);
	TRACE(relay,"(%p)->Release()\n",This);
	if (!--(This->ref)) {
		HeapFree(GetProcessHeap(),0,This);
		return 0;
	}
	return This->ref;
}

/******************************************************************************
 * IUnknown_QueryInterface [VTABLE:IUNKNOWN.0]
 */
static HRESULT WINAPI IUnknown_fnQueryInterface(LPUNKNOWN iface,REFIID refiid,LPVOID *obj) {
	ICOM_THIS(IUnknownImpl,iface);
	char	xrefiid[50];

	WINE_StringFromCLSID((LPCLSID)refiid,xrefiid);
	TRACE(relay,"(%p)->QueryInterface(%s,%p)\n",This,xrefiid,obj);

	if (!memcmp(&IID_IUnknown,refiid,sizeof(IID_IUnknown))) {
		*obj = This;
		return 0; 
	}
	return OLE_E_ENUM_NOMORE; 
}

static ICOM_VTABLE(IUnknown) uvt = {
	IUnknown_fnQueryInterface,
	IUnknown_fnAddRef,
	IUnknown_fnRelease
};

/******************************************************************************
 * IUnknown_Constructor [INTERNAL]
 */
LPUNKNOWN
IUnknown_Constructor() {
	IUnknownImpl*	unk;

	unk = (IUnknownImpl*)HeapAlloc(GetProcessHeap(),0,sizeof(IUnknownImpl));
	unk->lpvtbl	= &uvt;
	unk->ref	= 1;
	return (LPUNKNOWN)unk;
}


/* --- IMalloc16 implementation */


typedef struct
{
        /* IUnknown fields */
        ICOM_VTABLE(IMalloc16)* lpvtbl;
        DWORD                   ref;
        /* IMalloc16 fields */
        /* Gmm, I think one is not enough, we should probably manage a list of
         * heaps
 */
        HGLOBAL16 heap;
} IMalloc16Impl;

/******************************************************************************
 *		IMalloc16_QueryInterface	[COMPOBJ.500]
 */
HRESULT WINAPI IMalloc16_fnQueryInterface(IMalloc16* iface,REFIID refiid,LPVOID *obj) {
        ICOM_THIS(IMalloc16Impl,iface);
	char	xrefiid[50];

	WINE_StringFromCLSID((LPCLSID)refiid,xrefiid);
	TRACE(relay,"(%p)->QueryInterface(%s,%p)\n",This,xrefiid,obj);
	if (	!memcmp(&IID_IUnknown,refiid,sizeof(IID_IUnknown)) ||
		!memcmp(&IID_IMalloc,refiid,sizeof(IID_IMalloc))
	) {
		*obj = This;
		return 0;
	}
	return OLE_E_ENUM_NOMORE; 
}

/******************************************************************************
 *		IMalloc16_AddRef	[COMPOBJ.501]
 */
ULONG WINAPI IMalloc16_fnAddRef(IMalloc16* iface) {
        ICOM_THIS(IMalloc16Impl,iface);
	TRACE(relay,"(%p)->AddRef()\n",This);
	return 1; /* cannot be freed */
}

/******************************************************************************
 *		IMalloc16_Release	[COMPOBJ.502]
 */
ULONG WINAPI IMalloc16_fnRelease(IMalloc16* iface) {
        ICOM_THIS(IMalloc16Impl,iface);
	TRACE(relay,"(%p)->Release()\n",This);
	return 1; /* cannot be freed */
}

/******************************************************************************
 * IMalloc16_Alloc [COMPOBJ.503]
 */
LPVOID WINAPI IMalloc16_fnAlloc(IMalloc16* iface,DWORD cb) {
        ICOM_THIS(IMalloc16Impl,iface);
	TRACE(relay,"(%p)->Alloc(%ld)\n",This,cb);
	return (LPVOID)PTR_SEG_OFF_TO_SEGPTR(This->heap,LOCAL_Alloc(This->heap,0,cb));
}

/******************************************************************************
 * IMalloc16_Realloc [COMPOBJ.504]
 */
LPVOID WINAPI IMalloc16_fnRealloc(IMalloc16* iface,LPVOID pv,DWORD cb) {
        ICOM_THIS(IMalloc16Impl,iface);
	TRACE(relay,"(%p)->Realloc(%p,%ld)\n",This,pv,cb);
	return (LPVOID)PTR_SEG_OFF_TO_SEGPTR(This->heap,LOCAL_ReAlloc(This->heap,0,LOWORD(pv),cb));
}

/******************************************************************************
 * IMalloc16_Free [COMPOBJ.505]
 */
VOID WINAPI IMalloc16_fnFree(IMalloc16* iface,LPVOID pv) {
        ICOM_THIS(IMalloc16Impl,iface);
	TRACE(relay,"(%p)->Free(%p)\n",This,pv);
	LOCAL_Free(This->heap,LOWORD(pv));
}

/******************************************************************************
 * IMalloc16_GetSize [COMPOBJ.506]
 */
DWORD WINAPI IMalloc16_fnGetSize(const IMalloc16* iface,LPVOID pv) {
	ICOM_CTHIS(IMalloc16Impl,iface);
	TRACE(relay,"(%p)->GetSize(%p)\n",This,pv);
	return LOCAL_Size(This->heap,LOWORD(pv));
}

/******************************************************************************
 * IMalloc16_DidAlloc [COMPOBJ.507]
 */
INT16 WINAPI IMalloc16_fnDidAlloc(const IMalloc16* iface,LPVOID pv) {
        ICOM_CTHIS(IMalloc16,iface);
	TRACE(relay,"(%p)->DidAlloc(%p)\n",This,pv);
	return (INT16)-1;
}

/******************************************************************************
 * IMalloc16_HeapMinimize [COMPOBJ.508]
 */
LPVOID WINAPI IMalloc16_fnHeapMinimize(IMalloc16* iface) {
        ICOM_THIS(IMalloc16Impl,iface);
	TRACE(relay,"(%p)->HeapMinimize()\n",This);
	return NULL;
}

static ICOM_VTABLE(IMalloc16)* msegvt16 = NULL;

/******************************************************************************
 * IMalloc16_Constructor [VTABLE]
 */
LPMALLOC16
IMalloc16_Constructor() {
	IMalloc16Impl*	This;
        HMODULE16	hcomp = GetModuleHandle16("COMPOBJ");

	This = (IMalloc16Impl*)SEGPTR_NEW(IMalloc16Impl);
        if (!msegvt16) {
            This->lpvtbl = msegvt16 = SEGPTR_NEW(ICOM_VTABLE(IMalloc16));

#define VTENT(x) msegvt16->fn##x = (void*)WIN32_GetProcAddress16(hcomp,"IMalloc16_"#x);assert(msegvt16->fn##x)
            VTENT(QueryInterface);
            VTENT(AddRef);
            VTENT(Release);
            VTENT(Alloc);
            VTENT(Realloc);
            VTENT(Free);
            VTENT(GetSize);
            VTENT(DidAlloc);
            VTENT(HeapMinimize);
            msegvt16 = (ICOM_VTABLE(IMalloc16)*)SEGPTR_GET(msegvt16);
#undef VTENT
	}
	This->ref = 1;
	/* FIXME: implement multiple heaps */
	This->heap = GlobalAlloc16(GMEM_MOVEABLE,64000);
	LocalInit(This->heap,0,64000);
	return (LPMALLOC16)SEGPTR_GET(This);
}


/* --- IMalloc32 implementation */

typedef struct
{
        /* IUnknown fields */
        ICOM_VTABLE(IMalloc32)* lpvtbl;
        DWORD                   ref;
} IMalloc32Impl;

/******************************************************************************
 *		IMalloc32_QueryInterface	[VTABLE]
 */
static HRESULT WINAPI IMalloc32_fnQueryInterface(LPMALLOC32 iface,REFIID refiid,LPVOID *obj) {
	ICOM_THIS(IMalloc32Impl,iface);
	char	xrefiid[50];

	WINE_StringFromCLSID((LPCLSID)refiid,xrefiid);
	TRACE(relay,"(%p)->QueryInterface(%s,%p)\n",This,xrefiid,obj);
	if (	!memcmp(&IID_IUnknown,refiid,sizeof(IID_IUnknown)) ||
		!memcmp(&IID_IMalloc,refiid,sizeof(IID_IMalloc))
	) {
		*obj = This;
		return S_OK;
	}
	return OLE_E_ENUM_NOMORE; 
}

/******************************************************************************
 *		IMalloc32_AddRef	[VTABLE]
 */
static ULONG WINAPI IMalloc32_fnAddRef(LPMALLOC32 iface) {
	ICOM_THIS(IMalloc32Impl,iface);
	TRACE(relay,"(%p)->AddRef()\n",This);
	return 1; /* cannot be freed */
}

/******************************************************************************
 *		IMalloc32_Release	[VTABLE]
 */
static ULONG WINAPI IMalloc32_fnRelease(LPMALLOC32 iface) {
	ICOM_THIS(IMalloc32Impl,iface);
	TRACE(relay,"(%p)->Release()\n",This);
	return 1; /* cannot be freed */
}

/******************************************************************************
 * IMalloc32_Alloc [VTABLE]
 */
static LPVOID WINAPI IMalloc32_fnAlloc(LPMALLOC32 iface,DWORD cb) {
	ICOM_THIS(IMalloc32Impl,iface);
	TRACE(relay,"(%p)->Alloc(%ld)\n",This,cb);
	return HeapAlloc(GetProcessHeap(),0,cb);
}

/******************************************************************************
 * IMalloc32_Realloc [VTABLE]
 */
static LPVOID WINAPI IMalloc32_fnRealloc(LPMALLOC32 iface,LPVOID pv,DWORD cb) {
	ICOM_THIS(IMalloc32Impl,iface);
	TRACE(relay,"(%p)->Realloc(%p,%ld)\n",This,pv,cb);
	return HeapReAlloc(GetProcessHeap(),0,pv,cb);
}

/******************************************************************************
 * IMalloc32_Free [VTABLE]
 */
static VOID WINAPI IMalloc32_fnFree(LPMALLOC32 iface,LPVOID pv) {
	ICOM_THIS(IMalloc32Impl,iface);
	TRACE(relay,"(%p)->Free(%p)\n",This,pv);
	HeapFree(GetProcessHeap(),0,pv);
}

/******************************************************************************
 * IMalloc32_GetSize [VTABLE]
 */
static DWORD WINAPI IMalloc32_fnGetSize(const IMalloc32* iface,LPVOID pv) {
	ICOM_CTHIS(IMalloc32,iface);
	TRACE(relay,"(%p)->GetSize(%p)\n",This,pv);
	return HeapSize(GetProcessHeap(),0,pv);
}

/******************************************************************************
 * IMalloc32_DidAlloc [VTABLE]
 */
static INT32 WINAPI IMalloc32_fnDidAlloc(const IMalloc32* iface,LPVOID pv) {
	ICOM_CTHIS(IMalloc32Impl,iface);
	TRACE(relay,"(%p)->DidAlloc(%p)\n",This,pv);
	return -1;
}

/******************************************************************************
 * IMalloc32_HeapMinimize [VTABLE]
 */
static LPVOID WINAPI IMalloc32_fnHeapMinimize(LPMALLOC32 iface) {
	ICOM_THIS(IMalloc32Impl,iface);
	TRACE(relay,"(%p)->HeapMinimize()\n",This);
	return NULL;
}

static ICOM_VTABLE(IMalloc32) VT_IMalloc32 = {
    IMalloc32_fnQueryInterface,
    IMalloc32_fnAddRef,
  IMalloc32_fnRelease,
  IMalloc32_fnAlloc,
  IMalloc32_fnRealloc,
  IMalloc32_fnFree,
  IMalloc32_fnGetSize,
  IMalloc32_fnDidAlloc,
  IMalloc32_fnHeapMinimize
};

/******************************************************************************
 * IMalloc32_Constructor [VTABLE]
 */
LPMALLOC32
IMalloc32_Constructor() {
	IMalloc32Impl* This;

	This = (IMalloc32Impl*)HeapAlloc(GetProcessHeap(),0,sizeof(IMalloc32Impl));
	This->lpvtbl = &VT_IMalloc32;
	This->ref = 1;
	return (LPMALLOC32)This;
}

/****************************************************************************
 * API Functions
 */

/******************************************************************************
 *		IsValidInterface32	[OLE32.78]
 *
 * RETURNS
 *  True, if the passed pointer is a valid interface
 */
BOOL32 WINAPI IsValidInterface32(
	LPUNKNOWN punk	/* [in] interface to be tested */
) {
	return !(
		IsBadReadPtr32(punk,4)					||
		IsBadReadPtr32(punk->lpvtbl,4)				||
		IsBadReadPtr32(punk->lpvtbl->fnQueryInterface,9)	||
		IsBadCodePtr32(punk->lpvtbl->fnQueryInterface)
	);
}
