/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Aug 09 10:28:15 2000
 */
/* Compiler settings for featureextractor.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __featureextractor_h__
#define __featureextractor_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IFeatureExtractor_FWD_DEFINED__
#define __IFeatureExtractor_FWD_DEFINED__
typedef interface IFeatureExtractor IFeatureExtractor;
#endif 	/* __IFeatureExtractor_FWD_DEFINED__ */


#ifndef __FeatureExtractor_FWD_DEFINED__
#define __FeatureExtractor_FWD_DEFINED__

#ifdef __cplusplus
typedef class FeatureExtractor FeatureExtractor;
#else
typedef struct FeatureExtractor FeatureExtractor;
#endif /* __cplusplus */

#endif 	/* __FeatureExtractor_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_featureextractor_0000 */
/* [local] */ 

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_featureextractor_0000_0001
    {	FTYPE_ColorHsvHistogram256	= 1,//256
	FTYPE_ColorHsvHistogram64	= 2,//64
	FTYPE_ColorLabHistogram256	= 3,//256
	FTYPE_ColorLabHistogram64	= 4,//64
	FTYPE_ColorLuvHistogram256	= 5,//256
	FTYPE_ColorLuvHistogram64	= 6,//64
	FTYPE_ColorHsvMoment12	= 7,//6
	FTYPE_ColorHsvMoment123	= 8,//9
	FTYPE_ColorLabMoment12	= 9,//6
	FTYPE_ColorLabMoment123	= 10,//9
	FTYPE_ColorLuvMoment12	= 11,//6
	FTYPE_ColorLuvMoment123	= 12,//9
	FTYPE_ColorHsvCoherence64	= 13,//128
	FTYPE_ColorLabCoherence64	= 14,//128
	FTYPE_ColorLuvCoherence64	= 15,//128
	FTYPE_CoarsenessValue	= 16,//1
	FTYPE_CoarsenessVector	= 17,//10
	FTYPE_Contrast	= 18,//1
	FTYPE_Directionality	= 19,//8
	FTYPE_WaveletPwtTexture	= 20,//24
	FTYPE_WaveletTwtTexture	= 21,//104
	FTYPE_MRSAR	= 22,//15
	FTYPE_MAX	= 0//0
    }	IMAGE_FEATURE_TYPE;

typedef /* [public][public] */ 
enum __MIDL___MIDL_itf_featureextractor_0000_0002
    {	norNULL	= 0,
	norHistogram	= 0x1,
	norVectorLength	= 0x2
    }	NormalizeMethod;

#if 0 // This allows us to use BITMAPINFOHEADER in the IDL
typedef int BITMAPINFOHEADER;

#else
#include "wingdi.h"
#endif


extern RPC_IF_HANDLE __MIDL_itf_featureextractor_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_featureextractor_0000_v0_0_s_ifspec;

#ifndef __IFeatureExtractor_INTERFACE_DEFINED__
#define __IFeatureExtractor_INTERFACE_DEFINED__

/* interface IFeatureExtractor */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IFeatureExtractor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1EC5837A-FC89-421B-89B9-0D0816CBAE0B")
    IFeatureExtractor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ContainDIB( 
            /* [in] */ const BITMAPINFOHEADER __RPC_FAR *pbmi,
            /* [in] */ const BYTE __RPC_FAR *pbData,
            /* [in] */ const RECT __RPC_FAR *prc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeContainDIB( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Extract( 
            /* [in] */ IMAGE_FEATURE_TYPE ift,
            /* [in] */ NormalizeMethod nm,
            /* [out] */ float __RPC_FAR *prFeatureBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryFeatureLength( 
            IMAGE_FEATURE_TYPE ift,
            UINT __RPC_FAR *pcbLen) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IFeatureExtractorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IFeatureExtractor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IFeatureExtractor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IFeatureExtractor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ContainDIB )( 
            IFeatureExtractor __RPC_FAR * This,
            /* [in] */ const BITMAPINFOHEADER __RPC_FAR *pbmi,
            /* [in] */ const BYTE __RPC_FAR *pbData,
            /* [in] */ const RECT __RPC_FAR *prc);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeContainDIB )( 
            IFeatureExtractor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Extract )( 
            IFeatureExtractor __RPC_FAR * This,
            /* [in] */ IMAGE_FEATURE_TYPE ift,
            /* [in] */ NormalizeMethod nm,
            /* [out] */ float __RPC_FAR *prFeatureBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryFeatureLength )( 
            IFeatureExtractor __RPC_FAR * This,
            IMAGE_FEATURE_TYPE ift,
            UINT __RPC_FAR *pcbLen);
        
        END_INTERFACE
    } IFeatureExtractorVtbl;

    interface IFeatureExtractor
    {
        CONST_VTBL struct IFeatureExtractorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFeatureExtractor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFeatureExtractor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFeatureExtractor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFeatureExtractor_ContainDIB(This,pbmi,pbData,prc)	\
    (This)->lpVtbl -> ContainDIB(This,pbmi,pbData,prc)

#define IFeatureExtractor_DeContainDIB(This)	\
    (This)->lpVtbl -> DeContainDIB(This)

#define IFeatureExtractor_Extract(This,ift,nm,prFeatureBuffer)	\
    (This)->lpVtbl -> Extract(This,ift,nm,prFeatureBuffer)

#define IFeatureExtractor_QueryFeatureLength(This,ift,pcbLen)	\
    (This)->lpVtbl -> QueryFeatureLength(This,ift,pcbLen)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IFeatureExtractor_ContainDIB_Proxy( 
    IFeatureExtractor __RPC_FAR * This,
    /* [in] */ const BITMAPINFOHEADER __RPC_FAR *pbmi,
    /* [in] */ const BYTE __RPC_FAR *pbData,
    /* [in] */ const RECT __RPC_FAR *prc);


void __RPC_STUB IFeatureExtractor_ContainDIB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFeatureExtractor_DeContainDIB_Proxy( 
    IFeatureExtractor __RPC_FAR * This);


void __RPC_STUB IFeatureExtractor_DeContainDIB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFeatureExtractor_Extract_Proxy( 
    IFeatureExtractor __RPC_FAR * This,
    /* [in] */ IMAGE_FEATURE_TYPE ift,
    /* [in] */ NormalizeMethod nm,
    /* [out] */ float __RPC_FAR *prFeatureBuffer);


void __RPC_STUB IFeatureExtractor_Extract_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IFeatureExtractor_QueryFeatureLength_Proxy( 
    IFeatureExtractor __RPC_FAR * This,
    IMAGE_FEATURE_TYPE ift,
    UINT __RPC_FAR *pcbLen);


void __RPC_STUB IFeatureExtractor_QueryFeatureLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IFeatureExtractor_INTERFACE_DEFINED__ */



#ifndef __FEATUREEXTRACTORLib_LIBRARY_DEFINED__
#define __FEATUREEXTRACTORLib_LIBRARY_DEFINED__

/* library FEATUREEXTRACTORLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_FEATUREEXTRACTORLib;

EXTERN_C const CLSID CLSID_FeatureExtractor;

#ifdef __cplusplus

class DECLSPEC_UUID("85E6D07A-AD1F-4449-9F47-9304462E5932")
FeatureExtractor;
#endif
#endif /* __FEATUREEXTRACTORLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
