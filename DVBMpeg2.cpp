// DVBMpeg2.cpp : Implementation of DLL Exports.
/*
#include "stdafx.h"
#include "resource.h"
#include "DVBMpeg2.h"

class CDVBMpeg2Module : public CAtlDllModuleT< CDVBMpeg2Module >
{
public :
	DECLARE_LIBID(LIBID_DVBMpeg2Lib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DVBMPEG2, "{952892FB-8ED8-404F-8090-7CBF47762949}")
};

CDVBMpeg2Module _AtlModule;


// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}
/**/
#include "stdafx.h"
#include <fstream>
#include <set>
using namespace ATL;
using namespace tal;
using namespace std;

#include "resource.h"
#include <devctlpp.h>
#include <ByteStreampp.h>
#include <Sessionpp.h>

//#include "Mpeg2.h"
//#include "Structs.h"
#include "Remains.h"
//#include "IpPackets.h"

#include "..\include\iDVBMpeg2.h"
//подключить заголовок определени€ класса компонента
#include "Section.h"
#include "TSAnalyser.h"
#include "cDVBMPeg2.h"


/****/

/****/

//заголовок получаемый путем компил€ции файла x.h
#include "..\include\DVBMPeg2.h"
#include "..\include\DVBMPeg2_i.c"

#include "..\include\DVBMpeg2UI_i.c"
#include "..\DVBUI\DVBUI_i.c"

class CModule:
  public TComModule
{
private:
  TAL_BEGIN_OBJECT_MAP(LIBID_DVBMpeg2Lib)
	OBJECT_ENTRY(CLSID_DVBMpeg2, CDVBMpeg2)
  TAL_END_OBJECT_MAP()
public:
};
TAL_IMPLEMENT_INPROC_SERVER(CModule)
/**/