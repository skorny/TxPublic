// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently
/*
#pragma once

#ifndef STRICT
#define STRICT
#endif

 Modify the following defines if you have to target a platform prior to the ones specified below.
 Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400	// Change this to the appropriate value to target Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

 turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS


#include "resource.h"
#include "talbase.h"
#include <atlbase.h>
#include <atlcom.h>

using namespace ATL;
/**/

#include <talui.h>
#define PACKET_LENGHT 0xBC				//188
#define MAX_BUF_LENGHT 0x1024			//4132
//#define PACKET_LENGHT 0xc8		//200
//typedef enum{
//	IP					= 0x0000,					//IP ������
//	UNKNOWN			= 0x0001,					//����������� MPEG2 ������
//	DEM_UNKNOWN = 0x0002,					//���������������������� ����������� MPEG2 ������
//	TV					= 0x0003,					//������ ����������� � �����������
//}OUT_DESCRIPTION;
/**/