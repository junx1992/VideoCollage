/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Aug 09 10:28:15 2000
 */
/* Compiler settings for featureextractor.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;

extern const IID IID_IFeatureExtractor = {0x1EC5837A,0xFC89,0x421B,{0x89,0xB9,0x0D,0x08,0x16,0xCB,0xAE,0x0B}};

extern const IID LIBID_FEATUREEXTRACTORLib = {0x8458196A,0x2AB3,0x4873,{0xA0,0x59,0x2A,0x48,0x95,0xDE,0x66,0x73}};

extern const CLSID CLSID_FeatureExtractor = {0x85E6D07A,0xAD1F,0x4449,{0x9F,0x47,0x93,0x04,0x46,0x2E,0x59,0x32}};

#endif // CLSID_DEFINED

#ifdef __cplusplus
}
#endif

