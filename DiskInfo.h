#ifndef DISKINFO_H
#define DISKINFO_H

#include <windows.h>
#include <winbase.h>


#define VWIN32_DIOC_DOS_INT21     (1) 
#define VWIN32_DIOC_DOS_INT25     (2)
#define VWIN32_DIOC_DOS_INT26     (3)
#define VWIN32_DIOC_DOS_DRIVEINFO (6)

#define CARRY_FLAG   1

#define READSECTORS  0
#define WRITESECTORS 1 
#define SECTORSIZE   512
typedef struct _DIOC_REGISTERS{  
	DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
 	DWORD reg_Flags;
}DIOC_REGISTERS, *PDIOC_REGISTERS;

#pragma pack(1)
typedef struct _DISKIO {
    DWORD  dwStartSector;   // starting logical sector number
    WORD   wSectors;        // number of sectors
    DWORD  dwBuffer;        // address of read/write buffer
}DISKIO, *PDISKIO;
#pragma pack()


#define Win95    0  //Win95、Win3.1
#define Win98    1  //Win95OSR2、Win98
#define Win2000  2  //WinNT、Win2000

class CDiskInfo{
public:
	CDiskInfo();
	~CDiskInfo();
private:
	HANDLE hDev;
    DWORD dwCurrentPlatform;
    void GetPlatform();
    
	BOOL Win2000_AccessSectors(WORD CMD,BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff);
	BOOL Int25_ReadSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff);
    BOOL Int26_WriteSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff);
    BOOL Int21_AccessSectors(WORD CMD,BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff);

public:
	//对外统一提供Read和Write操作，类内部根据平台选用适合的函数调用
	BOOL ReadSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff);
	BOOL WriteSectors(BYTE bDrive,DWORD dwStartSector,WORD wSectors,LPBYTE lpSectBuff);
};

#endif
