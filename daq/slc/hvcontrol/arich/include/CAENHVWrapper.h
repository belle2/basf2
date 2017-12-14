/***************************************************************************/
/*                                                                         */
/*        --- CAEN Engineering Srl - Computing Systems Division ---        */
/*                                                                         */
/*    CAENHVWRAPPER.H                                                      */
/*                                                                         */
/*                                                                         */
/*    Source code written in ANSI C                                        */
/*                                                                         */
/*    Created:  July 2011                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef __CAENHVWRAPPER_H
#define __CAENHVWRAPPER_H

//#ifdef UNIX                           // Rel. 2.0 - Linux
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define CAENHVLIB_API

extern void Sleep(unsigned int x);

/*
#else // UNIX

//#include <windows.h>

#ifdef CAENHVLIB
#define CAENHVLIB_API __declspec(dllexport)
#else
#define CAENHVLIB_API
#endif

#endif // UNIX
*/

#ifdef __GNUC__
#define HV_DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define HV_DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: DEPRECATED marking not supported on this compiler")
#define HV_DEPRECATED(func) func
#endif

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef ushort
#define ushort unsigned short
#endif
#ifndef ulong
#define ulong unsigned int
#endif


#define MAX_CH_NAME                12

#define MAX_PARAM_NAME             10

#define MAX_CRATES                             8
#define MAX_SLOTS                             32
#define MAX_BOARDS    ( MAX_SLOTS * MAX_CRATES )

#define MAX_BOARD_NAME             12
#define MAX_BOARD_DESC             28
#define SET                         1
#define MON                         0
#define SIGNED                      1
#define UNSIGNED                    0

#define PARAM_TYPE_NUMERIC          0
#define PARAM_TYPE_ONOFF            1
#define PARAM_TYPE_CHSTATUS         2
#define PARAM_TYPE_BDSTATUS         3
#define PARAM_TYPE_BINARY     4     // Rel. 2.16
#define PARAM_TYPE_STRING     5
#define PARAM_TYPE_ENUM       6     // Rel 5.30

#define PARAM_MODE_RDONLY           0
#define PARAM_MODE_WRONLY           1
#define PARAM_MODE_RDWR             2

#define PARAM_UN_NONE               0
#define PARAM_UN_AMPERE             1
#define PARAM_UN_VOLT               2
#define PARAM_UN_WATT               3
#define PARAM_UN_CELSIUS            4
#define PARAM_UN_HERTZ              5
#define PARAM_UN_BAR                6
#define PARAM_UN_VPS                7
#define PARAM_UN_SECOND             8
#define PARAM_UN_RPM                9             // Rel. 1.4
#define PARAM_UN_COUNT             10             // Rel. 2.6
#define PARAM_UN_BIT               11

#define SYSPROP_TYPE_STR            0
#define SYSPROP_TYPE_REAL           1
#define SYSPROP_TYPE_UINT2          2
#define SYSPROP_TYPE_UINT4          3
#define SYSPROP_TYPE_INT2           4
#define SYSPROP_TYPE_INT4           5
#define SYSPROP_TYPE_BOOLEAN        6

#define SYSPROP_MODE_RDONLY         0
#define SYSPROP_MODE_WRONLY         1
#define SYSPROP_MODE_RDWR           2

#define EVENTTYPE_PARAMETER     0
#define EVENTTYPE_ALARM       1
#define EVENTTYPE_KEEPALIVE     2

#define MAXLINE           0x1000
#define MAX_AVAILABLE_DEVICE    100

/*-----------------------------------------------------------------------------
                             ERROR    CODES

  Their meaning is the next:
   CODES
     0    Command wrapper correctly executed
     1    Error of operatived system
     2    Write error in communication channel
     3    Read error in communication channel
     4    Time out in server communication
     5    Command Front End application is down
     6    Communication with system not yet connected by a Login command
   7    Communication with a not present board/slot
     8    Communication with RS232 not yet implemented
   9    User memory not sufficient
   10   Value out of range
   11   Execute command not yet implemented
     12   Get Property not yet implemented
     13   Set Property not yet implemented
   14   Property not found
   15   Execute command not found
   16   No System property
     17   No get property
     18   No set property
     19   No execute command
     20   Device configuration changed
   21   Property of param not found
     22   Param not found
   23   No data present
   24   Device already open
   25   To Many devices opened
   26   Function Parameter not valid
   27   Function not available for the connected device
   0x1001 Device already connected
   0x1002 Device not connected
   0x1003 Operating system error
   0x1004 Login failed
   0x1005 Logout failed
   0x1006 Link type not supported
   0x1007 Login failed for username/password ( SY4527 / SY5527 )
 -----------------------------------------------------------------------------*/
#define CAENHV_OK                   0
#define CAENHV_SYSERR               1
#define CAENHV_WRITEERR             2
#define CAENHV_READERR              3
#define CAENHV_TIMEERR              4
#define CAENHV_DOWN                 5
#define CAENHV_NOTPRES              6
#define CAENHV_SLOTNOTPRES          7
#define CAENHV_NOSERIAL             8
#define CAENHV_MEMORYFAULT          9
#define CAENHV_OUTOFRANGE           10
#define CAENHV_EXECCOMNOTIMPL       11
#define CAENHV_GETPROPNOTIMPL       12
#define CAENHV_SETPROPNOTIMPL       13
#define CAENHV_PROPNOTFOUND         14
#define CAENHV_EXECNOTFOUND         15
#define CAENHV_NOTSYSPROP       16
#define CAENHV_NOTGETPROP       17
#define CAENHV_NOTSETPROP           18
#define CAENHV_NOTEXECOMM           19
#define CAENHV_SYSCONFCHANGE      20
#define CAENHV_PARAMPROPNOTFOUND    21
#define CAENHV_PARAMNOTFOUND        22
#define CAENHV_NODATA       23
#define CAENHV_DEVALREADYOPEN   24
#define CAENHV_TOOMANYDEVICEOPEN  25
#define CAENHV_INVALIDPARAMETER   26
#define CAENHV_FUNCTIONNOTAVAILABLE 27
#define CAENHV_SOCKETERROR      28
#define CAENHV_COMMUNICATIONERROR 29
#define CAENHV_NOTYETIMPLEMENTED  30
#define CAENHV_CONNECTED      (0x1000 + 1)
#define CAENHV_NOTCONNECTED     (0x1000 + 2)
#define CAENHV_OS         (0x1000 + 3)
#define CAENHV_LOGINFAILED      (0x1000 + 4)
#define CAENHV_LOGOUTFAILED     (0x1000 + 5)
#define CAENHV_LINKNOTSUPPORTED   (0x1000 + 6)  // Rel. 1.2
#define CAENHV_USERPASSFAILED   (0x1000 + 7)  // Rel. 5.0

// Link Types for InitSystem
#define LINKTYPE_TCPIP        0
#define LINKTYPE_RS232        1
#define LINKTYPE_CAENET       2
#define LINKTYPE_USB        3
#define LINKTYPE_OPTLINK      4
#define LINKTYPE_USB_VCP      5

#ifndef __CAENHVRESULT__                         // Rel. 2.0 - Linux
// The Error Code type
typedef int CAENHVRESULT;
#define __CAENHVRESULT__
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef union {
  char      StringValue[1024];
  float     FloatValue;
  int       IntValue;
} IDValue_t;

typedef enum {
  PARAMETER   = 0,
  ALARM     = 1,
  KEEPALIVE   = 2
} CAENHV_ID_TYPE_t;

// Rel. 3.00
typedef struct {
  char  Type;
  char  ItemID[64];
  char  Lvalue[4];
  char  Tvalue[256];
} CAENHVEVENT_TYPE;

typedef struct {
  CAENHV_ID_TYPE_t  Type;
  int         SystemHandle;
  int         BoardIndex;
  int         ChannelIndex;
  char        ItemID[20];
  IDValue_t     Value;
} CAENHVEVENT_TYPE_t;

typedef enum {
  SY1527    = 0,
  SY2527    = 1,
  SY4527    = 2,
  SY5527    = 3,
  N568    = 4,
  V65XX   = 5,
  N1470   = 6,
  V8100   = 7,
  N568E   = 8,
  DT55XX    = 9
} CAENHV_SYSTEM_TYPE_t;

typedef enum {
  SYNC    = 0,
  ASYNC   = 1,
  UNSYNC    = 2,
  NOTAVAIL  = 3
} CAENHV_EVT_STATUS_t;

typedef struct {
  CAENHV_EVT_STATUS_t System;
  CAENHV_EVT_STATUS_t Board[16];
} CAENHV_SYSTEMSTATUS_t;

CAENHVLIB_API char* CAENHVLibSwRel(void);

CAENHVLIB_API CAENHVRESULT CAENHV_InitSystem(CAENHV_SYSTEM_TYPE_t system, int LinkType, void* Arg,
                                             const char* UserName, const char* Passwd,  int* handle);

CAENHVLIB_API CAENHVRESULT  CAENHV_DeinitSystem(int handle);

CAENHVLIB_API CAENHVRESULT CAENHV_GetCrateMap(int handle,
                                              ushort* NrOfSlot, ushort** NrofChList, char** ModelList, char** DescriptionList,
                                              ushort** SerNumList, uchar** FmwRelMinList, uchar** FmwRelMaxList);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetSysPropList(int handle,
                                                  ushort* NumProp, char** PropNameList);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetSysPropInfo(int handle,
                                                  const char* PropName, unsigned* PropMode, unsigned* PropType);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetSysProp(int handle,
                                              const char* PropName, void* Result);

CAENHVLIB_API CAENHVRESULT  CAENHV_SetSysProp(int handle,
                                              const char* PropName, void* Set);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetBdParam(int handle,
                                              ushort slotNum, const ushort* slotList, const char* ParName, void* ParValList);

CAENHVLIB_API CAENHVRESULT  CAENHV_SetBdParam(int handle,
                                              ushort slotNum, const ushort* slotList, const char* ParName, void* ParValue);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetBdParamProp(int handle,
                                                  ushort slot, const char* ParName, const char* PropName, void* retval);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetBdParamInfo(int handle,
                                                  ushort slot, char** ParNameList);

CAENHVLIB_API CAENHVRESULT  CAENHV_TestBdPresence(int handle,
                                                  ushort slot, ushort* NrofCh, char** Model, char** Description, ushort* SerNum,
                                                  uchar* FmwRelMin, uchar* FmwRelMax);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetChParamProp(int handle,
                                                  ushort slot, ushort Ch, const char* ParName, const char* PropName, void* retval);

CAENHVLIB_API CAENHVRESULT CAENHV_GetChParamInfo(int handle, ushort slot, ushort Ch,
                                                 char** ParNameList, int* ParNumber);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetChName(int handle, ushort slot,
                                             ushort ChNum, const ushort* ChList, char (*ChNameList)[MAX_CH_NAME]);

CAENHVLIB_API CAENHVRESULT  CAENHV_SetChName(int handle, ushort slot,
                                             ushort ChNum, const ushort* ChList, const char* ChName);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetChParam(int handle, ushort slot,
                                              const char* ParName, ushort ChNum, const ushort* ChList, void* ParValList);

CAENHVLIB_API CAENHVRESULT  CAENHV_SetChParam(int handle, ushort slot,
                                              const char* ParName, ushort ChNum, const ushort* ChList, void* ParValue);

CAENHVLIB_API CAENHVRESULT  CAENHV_GetExecCommList(int handle,
                                                   ushort* NumComm, char** CommNameList);

CAENHVLIB_API CAENHVRESULT  CAENHV_ExecComm(int handle, const char* CommName);

CAENHVLIB_API CAENHVRESULT CAENHV_SubscribeSystemParams(int handle, short Port, const char* paramNameList,
                                                        unsigned int paramNum , char* listOfResultCodes);

CAENHVLIB_API CAENHVRESULT CAENHV_SubscribeBoardParams(int handle, short Port, const unsigned short slotIndex,
                                                       const char* paramNameList, unsigned int paramNum , char* listOfResultCodes);

CAENHVLIB_API CAENHVRESULT CAENHV_SubscribeChannelParams(int handle, short Port, const unsigned short slotIndex,
                                                         const unsigned short chanIndex, const char* paramNameList,
                                                         unsigned int paramNum , char* listOfResultCodes);

CAENHVLIB_API CAENHVRESULT CAENHV_UnSubscribeSystemParams(int handle, short Port, const char* paramNameList,
                                                          unsigned int paramNum , char* listOfResultCodes);

CAENHVLIB_API CAENHVRESULT CAENHV_UnSubscribeBoardParams(int handle, short Port, const unsigned short slotIndex,
                                                         const char* paramNameList, unsigned int paramNum , char* listOfResultCodes);

CAENHVLIB_API CAENHVRESULT CAENHV_UnSubscribeChannelParams(int handle, short Port, const unsigned short slotIndex,
                                                           const unsigned short chanIndex, const char* paramNameList,
                                                           unsigned int paramNum , char* listOfResultCodes);

CAENHVLIB_API char* CAENHV_GetError(int handle);

#ifdef WIN32
CAENHVLIB_API CAENHVRESULT CAENHV_GetEventData(SOCKET sck, CAENHV_SYSTEMSTATUS_t* SysStatus,
                                               CAENHVEVENT_TYPE_t** EventData, unsigned int* DataNumber);
#else
CAENHVLIB_API CAENHVRESULT CAENHV_GetEventData(int sck, CAENHV_SYSTEMSTATUS_t* SysStatus,
                                               CAENHVEVENT_TYPE_t** EventData, unsigned int* DataNumber);
#endif

CAENHVLIB_API CAENHVRESULT CAENHV_FreeEventData(CAENHVEVENT_TYPE_t** ListOfItemsData);

CAENHVLIB_API CAENHVRESULT CAENHV_Free(void* arg);


/********************************************/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __CAENHVWRAPPER_H
