//#include "stdafx.h"
#include "DiskInfo.h"

CDiskInfo::CDiskInfo()
{
	GetPlatform();
	switch(dwCurrentPlatform)
	{
	case Win95:
		break;
	case Win98:
		hDev=CreateFileA("\\\\.\\vwin32",0,0,NULL,0,FILE_FLAG_DELETE_ON_CLOSE,NULL);
		break;
	case Win2000:
		break;
	}
}
CDiskInfo::~CDiskInfo()
{
	if(hDev!=INVALID_HANDLE_VALUE)CloseHandle(hDev);
}

void CDiskInfo::GetPlatform()
{
	OSVERSIONINFO osv;
    osv.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	BOOL bRet=GetVersionEx(&osv);
	if(bRet)
	{
		switch(osv.dwPlatformId)
		{
		case VER_PLATFORM_WIN32s:
			dwCurrentPlatform=Win95;
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if(osv.dwMinorVersion==0)dwCurrentPlatform=Win95;
			if(osv.dwMinorVersion>0)dwCurrentPlatform=Win98;
			break;
		case VER_PLATFORM_WIN32_NT:
			dwCurrentPlatform=Win2000;
			break;
		}
	}
}

BOOL CDiskInfo::Int25_ReadSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff)
{
      BOOL           fResult;
      DWORD          cb;
      DIOC_REGISTERS reg = {0};
      DISKIO         dio = {0};

      dio.dwStartSector = dwStartSector;
      dio.wSectors      = wSectors;
      dio.dwBuffer      = (DWORD)lpSectBuff;

      reg.reg_EAX = bDrive - 1;    // Int 25h drive numbers are 0-based.
      reg.reg_EBX = (DWORD)&dio;
      reg.reg_ECX = 0xFFFF;        // use DISKIO struct

      fResult = DeviceIoControl(hDev, VWIN32_DIOC_DOS_INT25,
                                &reg, sizeof(reg),
                                &reg, sizeof(reg), &cb, 0);

      // Determine if the DeviceIoControl call and the read succeeded.
      fResult = fResult && !(reg.reg_Flags & CARRY_FLAG);

      return fResult;
}   


BOOL CDiskInfo::Int26_WriteSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff)
{
 
	BOOL           fResult;
    DWORD          cb;
    DIOC_REGISTERS reg = {0};
    DISKIO         dio = {0};

	
    dio.dwStartSector = dwStartSector;
    dio.wSectors      = wSectors;
    dio.dwBuffer      = (DWORD)lpSectBuff;

    reg.reg_EAX = bDrive - 1;    // Int 26h drive numbers are 0-based.
    reg.reg_EBX = (DWORD)&dio;
    reg.reg_ECX = 0xFFFF;        // use DISKIO struct

    fResult = DeviceIoControl(hDev, VWIN32_DIOC_DOS_INT26,
                              &reg, sizeof(reg),
                              &reg, sizeof(reg), &cb, 0);

    // Determine if the DeviceIoControl call and the write succeeded.
    fResult = fResult && !(reg.reg_Flags & CARRY_FLAG);

    return fResult;
}


BOOL CDiskInfo::Int21_AccessSectors (WORD CMD,BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff)
{
	BOOL           fResult;
    DWORD          cb;
    DIOC_REGISTERS reg = {0};
    DISKIO         dio;

    dio.dwStartSector = dwStartSector;
    dio.wSectors      = wSectors;
    dio.dwBuffer      = (DWORD)lpSectBuff;

    reg.reg_EAX = 0x7305;   // Ext_ABSDiskReadWrite
    reg.reg_EBX = (DWORD)&dio;
    reg.reg_ECX = -1;
    reg.reg_EDX = bDrive;   // Int 21h, fn 7305h drive numbers are 1-based
	                        // 0 = default, 1 = A, 2 = B, 3 = C, etc.

	if(CMD==WRITESECTORS)
	{
		reg.reg_ESI = 0x6001;   
	}

    fResult = DeviceIoControl(hDev, VWIN32_DIOC_DOS_DRIVEINFO,
                              &reg, sizeof(reg),
                              &reg, sizeof(reg), &cb, 0);

    // Determine if the DeviceIoControl call and the read succeeded.
    fResult = fResult && !(reg.reg_Flags & CARRY_FLAG);

    return fResult;
}
BOOL CDiskInfo::Win2000_AccessSectors(WORD CMD,BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff)
{
	if(bDrive==0)return 0;
	DWORD dwAccess=0;
	BOOL bRet=0;
	DWORD dwCB;
	char devName[]="\\\\.\\A:";
	devName[4]='A'+bDrive-1;
	if(CMD==WRITESECTORS)dwAccess=GENERIC_WRITE;
    if(CMD==READSECTORS)dwAccess=GENERIC_READ;

	hDev=CreateFileA(devName,dwAccess,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	if(hDev==INVALID_HANDLE_VALUE)return 0;

	dwCB=SetFilePointer(hDev,SECTORSIZE*dwStartSector,0,FILE_BEGIN);
	switch(CMD)
	{
	case READSECTORS:
		bRet=ReadFile(hDev,lpSectBuff,SECTORSIZE*wSectors,&dwCB,NULL);
		break;
	case WRITESECTORS:
		bRet=WriteFile(hDev,lpSectBuff,SECTORSIZE*wSectors,&dwCB,NULL);
		break;
	}
	CloseHandle(hDev);
	return bRet;
}


BOOL CDiskInfo::ReadSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff)
{
	switch(dwCurrentPlatform)
	{
	case Win98:
		return Int21_AccessSectors(READSECTORS,bDrive,dwStartSector,wSectors,lpSectBuff);
	case Win95:
		break;
	case Win2000:
		return Win2000_AccessSectors(READSECTORS,bDrive,dwStartSector,wSectors,lpSectBuff);
	}
	return 0;
}
BOOL CDiskInfo::WriteSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff)
{
	switch(dwCurrentPlatform)
	{
	case Win95:
		break;
	case Win98:
		return Int21_AccessSectors(WRITESECTORS,bDrive,dwStartSector,wSectors,lpSectBuff);
	case Win2000:
		return Win2000_AccessSectors(WRITESECTORS,bDrive,dwStartSector,wSectors,lpSectBuff);
	}
	return 0;
}