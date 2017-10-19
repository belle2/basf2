// ~/slc$ g++ -Iinclude -DMPOD_MAIN hvcontrol/arichlv/src/MPOD.cc -lsnmp -o MPOD
// ~/slc$ g++ -Iinclude -DMPOD_MAIN hvcontrol/arichlv/src/MPOD.cc -lnetsnmp -o MPOD  #Ubuntu 16.04 LTS
// ~/public_html/mpod$ sudo -H -u www-data bash -c './MPOD 1'
// sudo cp ~/.snmp/mibs/WIENER-CRATE-MIB.txt /usr/share/snmp/mibs
#ifdef _WINDOWS
#include <windows.h>
#endif
#ifdef _CVI_
#include <cvirte.h>
#include <utility.h>
#endif
#include <stdio.h>
#include <stdarg.h>
//#include "MPOD.h"
#include <time.h>
#include "daq/slc/hvcontrol/arichlv/MPOD.h"


//************************* WIENER_SNMP.c *****************************
// WIENER SNMP basic SNMP library to Demonstrate C-Access to WIENER-Crates via SNMP
// modified for LabView import 04/23/06, Andreas Ruben
//
// The path to the Net-SNMP include files (default /usr/include) must be added to the
// include file search path!
// The following libraries must be included:
// netsnmp.lib ws2_32.lib
// The path to the Net-SNMP library must be added to the linker files.
// /usr/lib
// path for the WIENER MIB file (mibdirs) c:/usr/share/snmp/mibs

#ifdef _CVI_
#include "toolbox.h"
#include <ansi_c.h>
#else
#define Delay(x) usleep(1000000*(x))
#endif

#define VER_FILEVERSION             1,1,1,0
#define VER_FILEVERSION_STR         "1.1.1.0\0"

#ifdef _MSC_VER
#define strdup _strdup
#define vsnprintf vsprintf_s
#define strPrintf sprintf_s
#else
#ifdef _CVI_
#define strdup StrDup
#endif

#define strPrintf snprintf
#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

#define false 0;
#define true  1;


static const char WienerMibFileName[] = "WIENER-CRATE-MIB";
static const char DefaultReadCommunity[] = "public";  ///< default community name for read operations
static const char DefaultWriteCommunity[] = "guru";   ///< default community name for write operation

static char* m_readCommunity = (char*)DefaultReadCommunity;
static char* m_writeCommunity = (char*)DefaultWriteCommunity;

/**
 * @brief The SnmpObject class is used internally to resolve OIDs and for the SNMP calls.
 */
//class SnmpObject {
//public:
typedef struct {
  oid    id[MAX_OID_LEN]; ///< The resolved SNMP OID
  size_t len;             ///< The resolved OIDs length in byte
  char   desc[100];       ///< The OIDs textual representation, e.g. "sysDescr.0"
} SnmpObject;
//typedef struct snmp_object SnmpObject;

static SnmpObject moduleIndex[MaxSlotsPerCrate];
static SnmpObject moduleDescription[MaxSlotsPerCrate];
static SnmpObject moduleSupply[MaxModuleAuxSupplies][MaxSlotsPerCrate];
static SnmpObject moduleHardwareLimitVoltage[MaxSlotsPerCrate];
static SnmpObject moduleHardwareLimitCurrent[MaxSlotsPerCrate];
static SnmpObject moduleRampSpeedVoltage[MaxSlotsPerCrate];
static SnmpObject moduleRampSpeedCurrent[MaxSlotsPerCrate];
static SnmpObject moduleStatus[MaxSlotsPerCrate];
static SnmpObject moduleEventStatus[MaxSlotsPerCrate];
static SnmpObject moduleDoClear[MaxSlotsPerCrate];
static SnmpObject moduleAuxiliaryMeasurementTemperature[MaxModuleAuxTemperatures][MaxSlotsPerCrate];

static SnmpObject sysDescr;
static SnmpObject sysMainSwitch;
static SnmpObject sysStatus;
static SnmpObject sysVmeSysReset;
static SnmpObject outputNumber;
static SnmpObject groupsNumber;
static SnmpObject highVoltageGroupsSwitch;
static SnmpObject lowVoltageGroupsSwitch;
static SnmpObject ipStaticAddress;

static SnmpObject outputName[MaxChannelsPerCrate];
static SnmpObject outputIndex[MaxChannelsPerCrate];
static SnmpObject outputGroup[MaxChannelsPerCrate];
static SnmpObject outputStatus[MaxChannelsPerCrate];
static SnmpObject outputMeasurementSenseVoltage[MaxChannelsPerCrate];
static SnmpObject outputMeasurementTerminalVoltage[MaxChannelsPerCrate];
static SnmpObject outputMeasurementCurrent[MaxChannelsPerCrate];
static SnmpObject outputMeasurementTemperature[MaxChannelsPerCrate];
static SnmpObject outputSwitch[MaxChannelsPerCrate];
static SnmpObject outputUserConfig[MaxChannelsPerCrate];
static SnmpObject outputVoltage[MaxChannelsPerCrate];
static SnmpObject outputCurrent[MaxChannelsPerCrate];
static SnmpObject outputVoltageRiseRate[MaxChannelsPerCrate];
static SnmpObject outputVoltageFallRate[MaxChannelsPerCrate];
static SnmpObject outputCurrentRiseRate[MaxChannelsPerCrate];
static SnmpObject outputCurrentFallRate[MaxChannelsPerCrate];
static SnmpObject outputSupervisionBehavior[MaxChannelsPerCrate];
static SnmpObject outputSupervisionMinSenseVoltage[MaxChannelsPerCrate];
static SnmpObject outputSupervisionMaxSenseVoltage[MaxChannelsPerCrate];
static SnmpObject outputSupervisionMaxTerminalVoltage[MaxChannelsPerCrate];
static SnmpObject outputSupervisionMaxCurrent[MaxChannelsPerCrate];
static SnmpObject outputSupervisionMaxTemperature[MaxChannelsPerCrate];
static SnmpObject outputConfigMaxSenseVoltage[MaxChannelsPerCrate];
static SnmpObject outputConfigMaxTerminalVoltage[MaxChannelsPerCrate];
static SnmpObject outputConfigMaxCurrent[MaxChannelsPerCrate];
static SnmpObject outputSupervisionMaxPower[MaxChannelsPerCrate];
static SnmpObject outputTripTimeMaxCurrent[MaxChannelsPerCrate];

static SnmpObject sensorNumber;
static SnmpObject sensorTemperature[MaxSensors];
static SnmpObject sensorWarningThreshold[MaxSensors];
static SnmpObject sensorFailureThreshold[MaxSensors];

//static SnmpObject psFirmwareVersion;
static SnmpObject psSerialNumber;
static SnmpObject psOperatingTime;
static SnmpObject psDirectAccess;
static SnmpObject fanNumberOfFans;
static SnmpObject fanOperatingTime;
//static SnmpObject fanFirmwareVersion;
static SnmpObject fanSerialNumber;
static SnmpObject fanAirTemperature;
static SnmpObject fanSwitchOffDelay;
static SnmpObject fanNominalSpeed;
static SnmpObject fanSpeed[MaxFans];

static SnmpObject psAuxVoltage[MaxPsAuxSupplies];
static SnmpObject psAuxCurrent[MaxPsAuxSupplies];

static SnmpObject snmpCommunityName[MaxCommunities];

//static double snmpSetDouble(HSNMP session, const SnmpObject &object, double value);
static double snmpSetDouble(HSNMP session, const SnmpObject* object, double value);
static double snmpGetDouble(HSNMP session, const SnmpObject* object);
static int    snmpSetInt(HSNMP session, const SnmpObject* object, int value);
static int    snmpGetInt(HSNMP session, const SnmpObject* object);
static char*  snmpGetString(HSNMP session, const SnmpObject* object);

char snmpStringBuffer[1024];
char snmpLastErrorBuffer[1024];

SnmpDoubleBuffer snmpDoubleBuffer;
SnmpIntegerBuffer snmpIntegerBuffer;

// ************************************************************************
static int getNode(const char* const node, SnmpObject* object);
static int getIndexNode(const char* const nodeBase, int index, SnmpObject* object);



#ifdef MPOD_MAIN
float mabs(float a)
{
  if (a > 0) return a;
  else return -a;
}
int MPOD_CableTest(int id)
{
  double ret;
  int iret;
  int min = 0;
  int max = 3;
  const char  pathkek[100] = "arich-mpod3.b2nsm.kek.jp";
  const char  pathijs[100] = "f9mpod2.ijs.si";
  MPOD_Start();
  HSNMP crate =  MPOD_Open(pathkek);
  if (!crate) {
    crate =  MPOD_Open(pathijs);
    if (!crate) {
      printf("<h5>Error: Cannot connect to device %s</h5>\n", pathkek);
      return 0;
    }

  }

  time_t rawtime;
  struct tm* info;
  char buffer[80];

  time(&rawtime);

  info = localtime(&rawtime);
  printf("%s<br/>", asctime(info));




  if (id < 2)  for (int ch = 0; ch < 4; ch++) {
      setChannelSwitch(crate, ch , id);
      if (id) setChannelSwitch(crate, ch , 10);
      printf("setOutputSwitch ch=%d =>%d", ch, id);
      printf("<br/>\n");
    }
  else {

// set output voltage
    float vset[4];
    for (int ch = 0; ch < 4; ch++) {
      vset[ch] = ch + 1;
      setOutputVoltage(crate, ch , vset[ch]);

      char  name[0xFF], sch[0xFF];
      sprintf(sch, "%d", ch + 1);
      sprintf(name, "outputConfigMaxTerminalVoltage.%s", sch);
      ret = MPOD_SetDouble(crate, name, ch + 1.5);

      iret = getOutputUserConfig(crate, ch + 1);
      iret &= 0x17;
      setOutputUserConfig(crate, ch + 1 , iret);
    }




    printf("<table border=1 class='mytable' >");
    printf("<tr><th>Ch.</th><th>Vset</th><th>Vstatus</th><th>Imon</th><th>Vterminal</th><th>Vsense</th>\n");
    for (int ch = 0; ch < 4; ch++) printf("<th>Vterm%d</th><th>Sense%d</th>\n", ch + 1, ch + 1);
    for (int ch = 0; ch < 4; ch++) {
      printf("<tr><td>%d</td>\n", ch);

      for (int slot = min; slot < max; slot++) {
        int chid = ch + slot * 100;
        char  name[0xFF], sch[0xFF];
        sprintf(sch, "%d", chid + 1);
        if (slot == min) {

          ret = getOutputVoltage(crate, chid);
          printf("<td>%2.2f</td>", ret);

          iret = getChannelSwitch(crate, chid);
          int iret0 = getOutputUserConfig(crate, chid);
          printf("<td>%d:0x%x</td>", iret,  iret0);

          ret = getCurrentMeasurement(crate, chid);
          printf("<td>%f</td>\n", ret);


        }

        if (slot == min) {
          sprintf(name, "outputMeasurementTerminalVoltage.%s", sch);
          ret = MPOD_GetDouble(crate, name);
          printf("<td>%2.2f</td>", ret);

          sprintf(name, "outputMeasurementSenseVoltage.%s", sch);
          ret = MPOD_GetDouble(crate, name);
          if (mabs(vset[ch] - ret) > 0.1)
            printf("<td bgcolor='red' >%2.2f</td>\n", ret);
          else
            printf("<td bgcolor='green'>%2.2f</td>\n", ret);

        } else {

          for (int k = 0; k < 2; k++) {
            chid = ch + k * 4 + slot * 100;
            setOutputVoltage(crate, chid  , 0);
            setChannelSwitch(crate, chid  , 1);
            iret = getOutputUserConfig(crate, chid);
            iret &= 0x17;
            if (slot < 2) setOutputUserConfig(crate, chid  , iret);
            sprintf(sch, "%d", chid + 1);


            sprintf(name, "outputMeasurementTerminalVoltage.%s", sch);
            ret = MPOD_GetDouble(crate, name);
            printf("<td>%2.2f</td>", ret);


            sprintf(name, "outputMeasurementSenseVoltage.%s", sch);
            ret = MPOD_GetDouble(crate, name);
            if (mabs(vset[ch] - ret) > 0.1)
              printf("<td bgcolor='red' >%2.2f</td>\n", ret);
            else
              printf("<td bgcolor='green'>%2.2f</td>\n", ret);

          }


        }
      }
    }
    printf("</table>");
  }

  MPOD_Close(crate);
  MPOD_End();

  return 0;
}




#ifdef _WINDOWS
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpszCmdLine, int nCmdShow)
#else
int main(int argc , char**   argv)
#endif

{
#define MAX_CRATES 3
  HSNMP crates[MAX_CRATES];
  double ret;
//  double voltage;
  double vSet = 0;
  int ch = 200;
  int iret;
  char cret[2000];
  HSNMP crate;

#ifdef _CVIs_
  if (InitCVIRTE(hInstance, 0, 0) == 0) return -1;     /* out of memory */
#endif

  if (argc > 1) {
    MPOD_CableTest(atoi(argv[1]));
    return 0;
  }

  MPOD_Start();

  /*
    for (i=0;i<8;i++) {
      setChannelSwitch(crate, i, 0);
    setOutputVoltage(crate, i, 0.);
      setChannelSwitch(crate, 100+i, 0);
    setOutputVoltage(crate, 100+i, 0.);
    }
  */
  const char hosts[][255] = {"f9mpod2.ijs.si", "f9mpod.ijs.si", "arich-mpod1.kek.jp"};
  for (int i = 0; i < MAX_CRATES; i++) {
    crates[i] =  NULL;
    crates[i] =  MPOD_Open(hosts[i]);
    if (!crates[i]) continue;
    iret = getMainSwitch(crates[i]);
    printf("Main Switch Crate %d= %i\n", i, iret);
    if (!iret) continue;

    iret = MPOD_GetIntCh(crates[i], "fanNominalSpeed", 0);
    printf("Fan nominal speed = %i\n", iret);
    printf("Number of modules = %i\n", MPOD_GetIntCh(crates[i], "moduleNumber", 0));
    for (int module = 0; module < MaxSlotsPerCrate; module++) {
      printf("ModuleDescription crate %d module %d =>%s\n", i, module, getModuleDescription(crates[i], module));
    }
  }
  crate = crates[0];




//  iret=MPOD_GetInt(0,"moduleNumber.0");

  ret = MPOD_GetDouble(crate, "outputVoltage.201");
  printf("Output Voltage = %f.\n", ret);

  setOutputVoltage(crate, ch, 9000.);
//  vSet = getOutputVoltage(crate1, ch);
  vSet = MPOD_GetDoubleCh(crate, "outputVoltage", ch + 1);
  printf("Output Voltage %i = %f.\n", ch, vSet);

//Test Channel Status
  iret = getChannelSwitch(crate, ch);
  printf("Channel Status %i = %i\n", ch, iret);

//Test Reading the Sense Measurement
  ret = getOutputSenseMeasurement(crate, ch);
  printf("Sense Voltage =  %f\n", ret);

//Test Reading the Current
  ret = getCurrentMeasurement(crate, ch);
  printf("Current Measurement =  %f\n", ret);

  printf("Turning channel %i ON\n", ch);
  setChannelSwitch(crate, ch, 1);
  Delay(1);

//Test Channel Status
  iret = getChannelSwitch(crate, ch);
  printf("Channel Status %i = %i\n", ch, iret);

//Test Reading the Sense Measurement
  ret = getOutputSenseMeasurement(crate, ch);
  printf("Sense Voltage =  %f\n", ret);

//Test Reading the Current
  ret = getCurrentMeasurement(crate, ch);
  printf("Current Measurement =  %f\n", ret);

  getchar();

  printf("Turning channel %i OFF\n", ch);
  setChannelSwitch(crate, ch, 0);

  printf("-----------------------------------------------------------------\n");

  Delay(1);

  MPOD_Close(crate);
  MPOD_End();

  return 0;
}

#endif /* MPOD_MAIN */

int _VI_FUNC MPOD_Start(void)
{
  return (snmpInit());                     // basic init
}

HSNMP _VI_FUNC MPOD_Open(const char* address)
{
  return  snmpOpen(address);   // open TCP/IP socket
}

int _VI_FUNC MPOD_GetInt(HSNMP mpodn, const char* oidstr)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getNode(oidstr, &tmpObject);
  return snmpGetInt(mpodn, &tmpObject);
}

int _VI_FUNC MPOD_SetInt(HSNMP mpodn, const char* oidstr, int iset)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getNode(oidstr, &tmpObject);
  return snmpSetInt(mpodn, &tmpObject, iset);
}

double _VI_FUNC MPOD_GetDouble(HSNMP mpodn, const char* oidstr)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getNode(oidstr, &tmpObject);
  return snmpGetDouble(mpodn, &tmpObject);
}

double _VI_FUNC MPOD_SetDouble(HSNMP mpodn, const char* oidstr, double dset)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getNode(oidstr, &tmpObject);
  return snmpSetDouble(mpodn, &tmpObject, dset);
}

char* _VI_FUNC MPOD_GetString(HSNMP mpodn, const char* oidstr)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getNode(oidstr, &tmpObject);
  return snmpGetString(mpodn, &tmpObject);
}

int _VI_FUNC MPOD_GetIntCh(HSNMP mpodn, const char* oidstrbase, int ich)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getIndexNode(oidstrbase, ich, &tmpObject);
  return snmpGetInt(mpodn, &tmpObject);
}

int _VI_FUNC MPOD_SetIntCh(HSNMP mpodn, const char* oidstrbase, int ich, int iset)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getIndexNode(oidstrbase, ich, &tmpObject);
  return snmpSetInt(mpodn, &tmpObject, iset);
}

double _VI_FUNC MPOD_GetDoubleCh(HSNMP mpodn, const char* oidstrbase, int ich)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getIndexNode(oidstrbase, ich, &tmpObject);
  return snmpGetDouble(mpodn, &tmpObject);
}

double _VI_FUNC MPOD_SetDoubleCh(HSNMP mpodn, const char* oidstrbase, int ich,
                                 double dset)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getIndexNode(oidstrbase, ich, &tmpObject);
  return snmpSetDouble(mpodn, &tmpObject, dset);
}

char* _VI_FUNC MPOD_GetStringCh(HSNMP mpodn, const char* oidstrbase, int ich)
{
  SnmpObject tmpObject;
  if (!mpodn) return 0;
  getIndexNode(oidstrbase, ich, &tmpObject);
  return snmpGetString(mpodn, &tmpObject);
}

int _VI_FUNC MPOD_Close(HSNMP mpodn)
{
  if (!mpodn) return 0;
  snmpClose(mpodn);
  mpodn = NULL;
  return 0;
}

int _VI_FUNC MPOD_End(void)
{
  snmpCleanup();  // finish
  return 0;
}

//************************* WIENER_SNMP.c *****************************
#ifdef _MSC_VER

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  UNREFERENCED_PARAMETER(hModule);
  UNREFERENCED_PARAMETER(ul_reason_for_call);
  UNREFERENCED_PARAMETER(lpReserved);

  return TRUE;
}

#endif

/**
 * @brief Simple logging function with printf-like usage.
 * @internal
 * @param priority
 * @param format
 */
void sysLog(int priority, const char* format, ...)
{
  UNREFERENCED_PARAMETER(priority);

  va_list vaPrintf;
  va_start(vaPrintf, format);
  vprintf(format, vaPrintf);
  putchar('\n');

  // store errors in snmpLastErrorBuffer, which can be read by snmpGetLastError()
  if (priority == LOG_ERR)
    vsnprintf(snmpLastErrorBuffer, sizeof(snmpLastErrorBuffer), format, vaPrintf);

  va_end(vaPrintf);
}

// Helper functions

/**
 * @brief Resolves the OID from the textual SNMP description
 * and stores the OID in *object.
 * @internal
 * @param node e.g. "sysMainSwitch"
 * @param object the resolved OID
 * @return true on success, false otherwise
 */
static int getNode(const char* const node, SnmpObject* object)
{
  object->len = MAX_OID_LEN;
  if (!get_node(node, object->id, &object->len)) {
    snmp_log(LOG_ERR, "OID %s not found!\n", node);
    return false;
  }

#ifdef _MSC_VER
  strcpy_s(object->desc, sizeof(object->desc), node);
#else
  strncpy(object->desc, node, sizeof(object->desc));
#endif

  return true;
}

/**
 * @brief Resolves the OID from the textual SNMP description
 * with appended index and stores the OID in *object.
 * @internal
 * @param nodeBase e.g. "outputSwitch"
 * @param index e.g. 100
 * @param object the resolved OID
 * @return true on success, false otherwise
 */
static int getIndexNode(const char* const nodeBase, int index, SnmpObject* object)
{
  char node[100];

  strPrintf(node, sizeof(node), "%s.%i", nodeBase, index);

  return getNode(node, object);
}

/**
 * @brief Activates logging on stderr console.
 * @since 1.1
 * @note This is the default setting.
 */
void snmpSetStdErrLog(void)
{
  snmp_enable_stderrlog();
}

/**
 * @brief Activates logging to the file filename.
 * @since 1.1
 * @param fileName The full path to the file where all log
 * information should go to.
 * @note If the specified file already exists,
 * new log information is appended.
 */
void snmpSetFileLog(const char* const fileName)
{
  snmp_enable_filelog(fileName, 1);
}

/**
 * @brief Returns the library four digit version number as unsigned int value.
 *
 * This allows to check for a specific version number.
 * @since 1.1
 * @return The version number as unsigned long value, e.g. 0x01010000
 */
unsigned int snmpGetVersion(void)
{
  const uint32_t version[] = { VER_FILEVERSION };

  return (version[0] << 24) + (version[1] << 16) + (version[2] << 8) + version[3];
}

/**
 * @brief Returns the library four digit version number as null-terminated string.
 *
 * The digits are separated by dots.
 * @since 1.1
 * @return The version number, e.g. "1.1.0.3"
 */
const char* snmpGetVersionString(void)
{
  return VER_FILEVERSION_STR;
}

/**
 * @brief Setup the default conditions for logging and SNMP passwords.
 * @internal
 */
static void setDefaultSettings(void)
{
  snmpSetStdErrLog();

  m_readCommunity = (char*)DefaultReadCommunity;
  m_writeCommunity = (char*)DefaultWriteCommunity;

  memset(snmpLastErrorBuffer, 0, sizeof(snmpLastErrorBuffer));
}

/**
 * @brief SNMP Initialization.
 *
 * Resolves all needed OIDs from the MIB file and prepares the SNMP communication.
 * The actual connection to a MPOD crate is done with snmpOpen().
 * @return true on success, false otherwise (e.g. an OID could not be resolved)
 */
int snmpInit(void)
{
  setDefaultSettings();

  snmp_log(LOG_DEBUG, "*** Initialise SNMP ***\n");

  init_snmp("WIENER_SNMP_DLL");
  init_mib();               // init MIB processing
  if (!read_module(WienerMibFileName)) {    // read specific mibs
    snmp_log(LOG_ERR, "Unable to load SNMP MIB file \"%s\"\n", WienerMibFileName);
    return false;
  }
  snmp_log(LOG_DEBUG, "*** Translate OIDs ***\n");

  // Translate System OIDS
  getNode("sysDescr.0", &sysDescr); // FIXME: doesn't work atm in Linux

  if (
    (!getNode("sysMainSwitch.0",      &sysMainSwitch)) ||
    (!getNode("sysStatus.0",          &sysStatus)) ||
    (!getNode("sysVmeSysReset.0",     &sysVmeSysReset)) ||
    (!getNode("outputNumber.0",       &outputNumber)) ||
    (!getNode("groupsNumber.0",       &groupsNumber)) ||
    (!getNode("groupsSwitch.64",      &highVoltageGroupsSwitch)) ||
    (!getNode("groupsSwitch.128",     &lowVoltageGroupsSwitch)) ||
    (!getNode("ipStaticAddress.0",    &ipStaticAddress)) ||
//    (!getNode("psFirmwareVersion.0",  &psFirmwareVersion)) ||
    (!getNode("psSerialNumber.0",     &psSerialNumber)) ||
    (!getNode("psOperatingTime.0",    &psOperatingTime)) ||
    (!getNode("psDirectAccess.0",     &psDirectAccess)) ||
    (!getNode("sensorNumber.0",       &sensorNumber)) ||
//    (!getNode("fanFirmwareVersion.0", &fanFirmwareVersion)) ||
    (!getNode("fanSerialNumber.0",    &fanSerialNumber)) ||
    (!getNode("fanOperatingTime.0",   &fanOperatingTime)) ||
    (!getNode("fanAirTemperature.0",  &fanAirTemperature)) ||
    (!getNode("fanSwitchOffDelay.0",  &fanSwitchOffDelay)) ||
    (!getNode("fanNominalSpeed.0",    &fanNominalSpeed)) ||
    (!getNode("fanNumberOfFans.0",    &fanNumberOfFans))
  ) {
    return false;
  }

  // Translate module and channel information OIDs
  for (int slot = 0; slot < MaxSlotsPerCrate; ++slot) {

    if (
      (!getIndexNode("moduleIndex", slot + 1, &moduleIndex[slot])) ||
      (!getIndexNode("moduleDescription", slot + 1, &moduleDescription[slot])) ||
      (!getIndexNode("moduleAuxiliaryMeasurementVoltage0", slot + 1, &moduleSupply[0][slot])) ||
      (!getIndexNode("moduleAuxiliaryMeasurementVoltage1", slot + 1, &moduleSupply[1][slot])) ||
      (!getIndexNode("moduleAuxiliaryMeasurementTemperature0", slot + 1, &moduleAuxiliaryMeasurementTemperature[0][slot])) ||
      (!getIndexNode("moduleAuxiliaryMeasurementTemperature1", slot + 1, &moduleAuxiliaryMeasurementTemperature[1][slot])) ||
      (!getIndexNode("moduleAuxiliaryMeasurementTemperature2", slot + 1, &moduleAuxiliaryMeasurementTemperature[2][slot])) ||
      (!getIndexNode("moduleAuxiliaryMeasurementTemperature3", slot + 1, &moduleAuxiliaryMeasurementTemperature[3][slot])) ||
      (!getIndexNode("moduleHardwareLimitVoltage", slot + 1, &moduleHardwareLimitVoltage[slot])) ||
      (!getIndexNode("moduleHardwareLimitCurrent", slot + 1, &moduleHardwareLimitCurrent[slot])) ||
      (!getIndexNode("moduleRampSpeedVoltage", slot + 1, &moduleRampSpeedVoltage[slot])) ||
      (!getIndexNode("moduleRampSpeedCurrent", slot + 1, &moduleRampSpeedCurrent[slot])) ||
      (!getIndexNode("moduleStatus", slot + 1, &moduleStatus[slot])) ||
      (!getIndexNode("moduleEventStatus", slot + 1, &moduleEventStatus[slot])) ||
      (!getIndexNode("moduleDoClear", slot + 1, &moduleDoClear[slot]))
    ) {
      return false;
    }

    int base = MaxChannelsPerSlot * slot; // array index

    for (int channel = base; channel < base + MaxChannelsPerSlot; ++channel) {
      if (
        (!getIndexNode("outputUserConfig", channel + 1, &outputUserConfig[channel])) ||
        (!getIndexNode("outputName", channel + 1, &outputName[channel])) ||
        (!getIndexNode("outputIndex", channel + 1, &outputIndex[channel])) ||
        (!getIndexNode("outputGroup", channel + 1, &outputGroup[channel])) ||
        (!getIndexNode("outputStatus", channel + 1, &outputStatus[channel])) ||
        (!getIndexNode("outputMeasurementSenseVoltage", channel + 1, &outputMeasurementSenseVoltage[channel])) ||
        (!getIndexNode("outputMeasurementTerminalVoltage", channel + 1, &outputMeasurementTerminalVoltage[channel])) ||
        (!getIndexNode("outputMeasurementCurrent", channel + 1, &outputMeasurementCurrent[channel])) ||
        (!getIndexNode("outputMeasurementTemperature", channel + 1, &outputMeasurementTemperature[channel])) ||
        (!getIndexNode("outputSwitch", channel + 1, &outputSwitch[channel])) ||
        (!getIndexNode("outputVoltage", channel + 1, &outputVoltage[channel])) ||
        (!getIndexNode("outputCurrent", channel + 1, &outputCurrent[channel])) ||
        (!getIndexNode("outputVoltageRiseRate", channel + 1, &outputVoltageRiseRate[channel])) ||
        (!getIndexNode("outputVoltageFallRate", channel + 1, &outputVoltageFallRate[channel])) ||
        (!getIndexNode("outputCurrentRiseRate", channel + 1, &outputCurrentRiseRate[channel])) ||
        (!getIndexNode("outputCurrentFallRate", channel + 1, &outputCurrentFallRate[channel])) ||
        (!getIndexNode("outputSupervisionBehavior", channel + 1, &outputSupervisionBehavior[channel])) ||
        (!getIndexNode("outputSupervisionMinSenseVoltage", channel + 1, &outputSupervisionMinSenseVoltage[channel])) ||
        (!getIndexNode("outputSupervisionMaxSenseVoltage", channel + 1, &outputSupervisionMaxSenseVoltage[channel])) ||
        (!getIndexNode("outputSupervisionMaxTerminalVoltage", channel + 1, &outputSupervisionMaxTerminalVoltage[channel])) ||
        (!getIndexNode("outputSupervisionMaxCurrent", channel + 1, &outputSupervisionMaxCurrent[channel])) ||
//        (!getIndexNode("outputSupervisionMaxTemperature", channel + 1, &outputSupervisionMaxTemperature[channel])) ||
        (!getIndexNode("outputConfigMaxSenseVoltage", channel + 1, &outputConfigMaxSenseVoltage[channel])) ||
        (!getIndexNode("outputConfigMaxTerminalVoltage", channel + 1, &outputConfigMaxTerminalVoltage[channel])) ||
        (!getIndexNode("outputSupervisionMaxPower", channel + 1, &outputSupervisionMaxPower[channel])) ||
        (!getIndexNode("outputConfigMaxCurrent", channel + 1, &outputConfigMaxCurrent[channel])) ||
        (!getIndexNode("outputTripTimeMaxCurrent", channel + 1, &outputTripTimeMaxCurrent[channel]))
      ) {
        return false;
      }
    }
  }

  for (int sensor = 0; sensor < MaxSensors; ++sensor)
    if (
      (!getIndexNode("sensorTemperature", sensor + 1, &sensorTemperature[sensor])) ||
      (!getIndexNode("sensorWarningThreshold", sensor + 1, &sensorWarningThreshold[sensor])) ||
      (!getIndexNode("sensorFailureThreshold", sensor + 1, &sensorFailureThreshold[sensor]))
    ) {
      return false;
    }

  for (int name = 0; name < MaxCommunities; ++name)
    if (!getIndexNode("snmpCommunityName", name + 1, &snmpCommunityName[name]))
      return false;

  for (int fan = 0; fan < MaxFans; ++fan)
    if (!getIndexNode("fanSpeed", fan + 1, &fanSpeed[fan]))
      return false;

  for (int aux = 0; aux < MaxPsAuxSupplies; ++aux) {
    if (
      (!getIndexNode("psAuxiliaryMeasurementVoltage", aux + 1, &psAuxVoltage[aux])) ||
      (!getIndexNode("psAuxiliaryMeasurementCurrent", aux + 1, &psAuxCurrent[aux]))
    ) {
      return false;
    }
  }

  snmp_log(LOG_DEBUG, "*** Initialise SNMP done ***\n");
  SOCK_STARTUP;                     // only in main thread

  return true;
}

/**
 * @brief Additional cleanup. Should be called after snmpClose.
 */
void snmpCleanup(void)
{
  SOCK_CLEANUP;
}

/**
 * @brief Set a new read community name for SNMP access.
 *
 * The read community name has to match the configured read community name in the MPOD.
 * The default read community name is "public".
 * @since 1.1
 * @note This function must be called before snmpOpen().
 * @param readCommunityName the new read community name
 */
void snmpSetReadCommunityName(const char* const readCommunityName)
{
  m_readCommunity = strdup(readCommunityName);
}

/**
 * @brief Set a new write community name for SNMP access.
 *
 * The write community name has to match the configured write community name in the MPOD.
 * The default write community name is "guru".
 * @since 1.1
 * @note This function must be called before any write access function.
 * @param writeCommunityName the new write community name
 */
void snmpSetWriteCommunityName(const char* const writeCommunityName)
{
  m_writeCommunity = strdup(writeCommunityName);
}

/**
 * @brief Opens a SNMP session to the specified ipAddress.
 *
 * This function also sets the number of retries and the timeout value.
 * @param ipAddress a zero-terminated ASCII string representation
 * of an IPv4 address, e.g. "192.168.17.101"
 * @return a handle to the opened SNMP session, which is a required
 * parameter for any further call.
 */
HSNMP snmpOpen(const char* const ipAddress)
{
  HSNMP session;
  struct snmp_session snmpSession;
  snmp_sess_init(&snmpSession);                  // structure defaults
  snmpSession.version = SNMP_VERSION_2c;
  snmpSession.peername = strdup(ipAddress);
  /*
    snmpSession.community = (u_char*)strdup(m_readCommunity);
    snmpSession.community_len = strlen(m_readCommunity);
  */
  snmpSession.community = (u_char*)strdup(m_writeCommunity);
  snmpSession.community_len = strlen(m_writeCommunity);

  snmpSession.timeout = 300000;   // timeout (us)
  snmpSession.retries = 2;        // retries

  if (!(session = snmp_sess_open(&snmpSession))) {
    int liberr, syserr;
    char* errstr;
    snmp_error(&snmpSession, &liberr, &syserr, &errstr);
    snmp_log(LOG_ERR, "Open SNMP session for host \"%s\": %s\n", ipAddress, errstr);
    //free(errstr);
    return 0;
  }

  snmp_log(LOG_INFO, "SNMP session for host \"%s\" opened\n", ipAddress);
  return session;
}

/**
 * @brief Closes the previously opened session specified by session.
 * @param session The handle returned by snmpOpen()
 */
void snmpClose(HSNMP session)
{
  if (!session)
    return;

  if (!snmp_sess_close(session))
    snmp_log(LOG_ERR, "Close SNMP session: ERROR\n");
  else
    snmp_log(LOG_INFO, "SNMP session closed\n");
}

/**
 * @brief Returns a pointer to a descriptive string for the last failed SNMP operation.
 * @return a pointer to a zero-terminated error string for the last failed
 * SNMP operation. Note: this pointer is valid until the next string operation.
 */
char* snmpGetLastError(void)
{
  return snmpLastErrorBuffer;
}

// System Information Functions

/**
 * @brief Returns a pointer to the MPOD controller description string.
 *
 * The pointer is valid until the next call of any string function.
 * @param session The handle returned by snmpOpen()
 * @return the MPOD controller description string, containing the
 * controller serial number and firmware releases, e.g.:
 * "WIENER MPOD (4388090, MPOD 2.1.2098.1, MPODslave 1.09, MPOD-BL 1.50 )"
 */
char* getSysDescr(HSNMP session)
{
  return snmpGetString(session, &sysDescr);
}

/**
 * @brief Returns the crate power on/off status.
 *
 * The result is the logical "and" between the hardware main switch
 * and the setMainSwitch function.
 * @param session The handle returned by snmpOpen()
 * @return The current on/off status of the crate:
 * 0: crate is powered off
 * 1: crate is powered on
 */
int getMainSwitch(HSNMP session)
{
  return snmpGetInt(session, &sysMainSwitch);
}

/**
 * @brief Sets the crate main switch to 1 = on or 0 = off.
 *
 * If the hardware main switch is set to "0" position, this function always returns 0.
 * @param session The handle returned by snmpOpen()
 * @param value 0 = set off, 1 = set on
 * @return The new on/off status of the crate.
 */
int setMainSwitch(HSNMP session, int value)
{
  return snmpSetInt(session, &sysMainSwitch, value);
}

/**
 * @brief Returns a bit field with the status of the complete crate.
 * @param session The handle returned by snmpOpen()
 * @return The complete crate status.
 */
int getMainStatus(HSNMP session)
{
  return snmpGetInt(session, &sysStatus);
}

/**
 * @brief Returns the VME system reset status.
 * @param session The handle returned by snmpOpen()
 * @return
 */
int getVmeReset(HSNMP session)
{
  return snmpGetInt(session, &sysVmeSysReset);
}

/**
 * @brief Initiate a VME system reset.
 * @param session The handle returned by snmpOpen()
 * @return
 */
int setVmeReset(HSNMP session)
{
  return snmpSetInt(session, &sysVmeSysReset, 1);
}

/**
 * @brief Returns the static IP address as 32 bit integer.
 * @param session The handle returned by snmpOpen()
 * @return The static IP address.
 */
int getIpStaticAddress(HSNMP session)
{
  return snmpGetInt(session, &ipStaticAddress);
}

/**
 * @brief Sets a new static IP address.
 * @param session The handle returned by snmpOpen()
 * @param value The IP address as 32 bit integer
 * @return
 */
int setIpStaticAddress(HSNMP session, int value)
{
  return snmpSetInt(session, &ipStaticAddress, value);
}

/**
 * @brief Returns a pointer to a string containing the MPOD controllers serial number.
 *
 * The pointer is valid until the next call of any string function.
 * @param session The handle returned by snmpOpen()
 * @return The crates serial number, e.g. "4388090".
 */
char* getPsSerialNumber(HSNMP session)
{
  return snmpGetString(session, &psSerialNumber);
}

// System Count Functions

/**
 * @brief Returns the total number of output channels in the crate.
 * @param session The handle returned by snmpOpen()
 * @return The total number of output channels
 */
int getOutputNumber(HSNMP session)
{
  return snmpGetInt(session, &outputNumber);
}

/**
 * @brief getOutputGroups
 * @param session The handle returned by snmpOpen()
 * @return
 */
int getOutputGroups(HSNMP session)
{
  return snmpGetInt(session, &groupsNumber);
}

// Output Channel Information

/**
 * @brief getOutputGroup
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
int getOutputGroup(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputGroup[channel]);
}

/**
 * @brief Returns the channel outputStatus register.
 * @note This function is deprecated. Use getOutputStatus() instead.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The channels outputStatus register
 */
int getChannelStatus(HSNMP session, int channel)
{
  return getOutputStatus(session, channel);
}

/**
 * @brief Returns the channel outputStatus register.
 * @since 1.1
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The channels outputStatus register
 */
int getOutputStatus(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputStatus[channel]);
}


/**
 * @brief Returns the channel outputUserConfig register.
 * @since 1.1
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The channels outputStatus register
 */
int getOutputUserConfig(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputUserConfig[channel]);
}


/**
 * @brief Returns the measured output sense voltage for channel in Volt.
 * @note This is only valid for WIENER LV modules.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputSenseMeasurement(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputMeasurementSenseVoltage[channel]);
}

/**
 * @brief Returns the measured output terminal voltage for channel in Volt.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The measured output terminal voltage in Volt.
 */
double getOutputTerminalMeasurement(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputMeasurementTerminalVoltage[channel]);
}

/**
 * @brief Returns the measured output current for channel in Ampere.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The measured output measurement current in Ampere.
 */
double getCurrentMeasurement(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputMeasurementCurrent[channel]);
}

/**
 * @brief Returns the measured temperature for channel in Degree Celsius.
 * @note Only WIENER Low Voltage modules have a channel-wise temperature measurement.
 * For iseg HV modules, use getModuleAuxTemperature().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The measured output temperature in Degree Celsius.
 */
int getTemperatureMeasurement(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputMeasurementTemperature[channel]);
}

/**
 * @brief Change the state of the channel.
 * @note This function is deprecated. Use setOutputSwitch() instead.
 * @param session The handle returned by snmpOpen()
 * @param channel The channel in the range of 0...999
 * @param value One of the following: off (0), on (1),
 * resetEmergencyOff (2), setEmergencyOff (3), clearEvents (10).
 * @return
 */
int setChannelSwitch(HSNMP session, int channel, int value)
{
  return setOutputSwitch(session, channel, value);
}

/**
 * @brief Change the state of the channel.
 * @since 1.1
 * @param session The handle returned by snmpOpen()
 * @param channel The channel in the range of 0...999
 * @param value One of the following: off (0), on (1),
 * resetEmergencyOff (2), setEmergencyOff (3), clearEvents (10).
 * @return
 */
int setOutputSwitch(HSNMP session, int channel, int value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate) return 0;
  return snmpSetInt(session, &outputSwitch[channel], value);
}

/**
 * @brief Change the outputUserConfig of the channel
 * @param session The handle returned by snmpOpen()
 * @param channel The channel in the range of 0...999
 * @param value bitvalue:A bit field packed into an integer which define the behavior of the output channel.
             Usable for WIENER LV devices only.
             The position of the bit fields in the integer value are:
             Bit 0:    Voltage ramping at switch off:
                           0: Ramp down at switch off.
                           1: No ramp at switch off (immediate off)
             Bit 1, 2: Set different regulation modes, dependent on the
                           cable inductance:
                     0: fast:     short cables, up to 1 meter.
                     1: moderate: cables from 1 to 30 meter.
                     2: slow:     cables longer than 30 meter.
                     3: slow (identical to 2, should not be used)
             Bit 3:    Internal sense line connection to the output (MPOD only):
                           0: The sense input at the sense connector is used
                              for regulation.
                           1: The output voltage is used for regulation.
                              Any signals at the sense connector are ignored.
             Bit 4:    External Inhibit input.
                           0: The external inhibit input is ignored.
                           1: The external inhibit input must be connected to
                              a voltage source to allow switch on.
 * @return
 */
int setOutputUserConfig(HSNMP session, int channel, int value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate) return 0;
  return snmpSetInt(session, &outputUserConfig[channel], value);
}



/**
 * @brief Returns the state of the channel.
 * @note This function is deprecated. Use getOutputSwitch() instead.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return One of the following: off (0), on (1),
 * resetEmergencyOff (2), setEmergencyOff (3), clearEvents (10).
 */
int getChannelSwitch(HSNMP session, int channel)
{
  return getOutputSwitch(session, channel);
}

/**
 * @brief Returns the state of the channel.
 * @since 1.1
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return One of the following: off (0), on (1),
 * resetEmergencyOff (2), setEmergencyOff (3), clearEvents (10).
 */
int getOutputSwitch(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputSwitch[channel]);
}

/**
 * @brief setHighVoltageGroupsSwitch
 * @param session The handle returned by snmpOpen()
 * @param value
 * @return
 */
int setHighVoltageGroupsSwitch(HSNMP session, int value)
{
  return snmpSetInt(session, &highVoltageGroupsSwitch, value);
}

/**
 * @brief getHighVoltageGroupsSwitch
 * @param session The handle returned by snmpOpen()
 * @return
 */
int getHighVoltageGroupsSwitch(HSNMP session)
{
  return snmpGetInt(session, &highVoltageGroupsSwitch);
}

/**
 * @brief setLowVoltageGroupsSwitch
 * @param session The handle returned by snmpOpen()
 * @param value
 * @return
 */
int setLowVoltageGroupsSwitch(HSNMP session, int value)
{
  return snmpSetInt(session, &lowVoltageGroupsSwitch, value);
}

/**
 * @brief getLowVoltageGroupsSwitch
 * @param session The handle returned by snmpOpen()
 * @return
 */
int getLowVoltageGroupsSwitch(HSNMP session)
{
  return snmpGetInt(session, &lowVoltageGroupsSwitch);
}

/**
 * @brief Returns the demanded output voltage for channel.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The demanded output voltage in Volt.
 */
double getOutputVoltage(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputVoltage[channel]);
}

/**
 * @brief Sets the demanded output voltage for channel.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @param value the demanded output voltage in Volt.
 * @return The demanded output voltage in Volt.
 */
double setOutputVoltage(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputVoltage[channel], value);
}

/**
 * @brief Returns the demanded maximum output current for channel.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The demanded output current in Ampere.
 */
double getOutputCurrent(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputCurrent[channel]);
}

/**
 * @brief Sets the demanded maximum output current for channel.
 * @param session The handle returned by snmpOpen()
 * @param channel The channel in the range of 0...999
 * @param value The demanded ouput current in Ampere
 * @return The demanded maximum output current in Ampere.
 */
double setOutputCurrent(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputCurrent[channel], value);
}

/**
 * @brief Returns the channel voltage rise rate in Volt/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use getModuleRampSpeedVoltage().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputRiseRate(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputVoltageRiseRate[channel]);
}

/**
 * @brief Sets the channel voltage rise rate in Volt/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use setModuleRampSpeedVoltage().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @param value
 * @return
 */
double setOutputRiseRate(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputVoltageRiseRate[channel], value);
}

/**
 * @brief Returns the channel voltage fall rate in Volt/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use getModuleRampSpeedVoltage().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputFallRate(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputVoltageFallRate[channel]);
}

/**
 * @brief Sets the channel voltage fall rate in Volt/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use setModuleRampSpeedVoltage().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @param value
 * @return
 */
double setOutputFallRate(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputVoltageFallRate[channel], value);
}

/**
 * @brief Returns the channel current rise rate in Ampere/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use getModuleRampSpeedCurrent().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputCurrentRiseRate(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputCurrentRiseRate[channel]);
}

/**
 * @brief Sets the channel current rise rate in Ampere/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use setModuleRampSpeedCurrent().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @param value
 * @return
 */
double setOutputCurrentRiseRate(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputCurrentRiseRate[channel], value);
}

/**
 * @brief Returns the channel current fall rate in Ampere/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use getModuleRampSpeedCurrent().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputCurrentFallRate(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputCurrentFallRate[channel]);
}

/**
 * @brief Sets the channel current fall rate in Ampere/second.
 *
 * @note This function is for WIENER LV only.
 * For iseg HV modules, use setModuleRampSpeedCurrent().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @param value
 * @return
 */
double setOutputCurrentFallRate(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputCurrentFallRate[channel], value);
}

/**
 * @brief Returns a bit field packed into an integer
 * which define the behavior of the output channel or power supply after failures.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
int getOutputSupervisionBehavior(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputSupervisionBehavior[channel]);
}

/**
 * @brief Set the behavior of the output channel or power supply after failures.
 *
 * For each supervision value, a two-bit field exists.
 * The enumeration of this value (..L+..H*2) is:
 *           WIENER LV devices
 *              0           ignore the failure
 *              1           switch off this channel
 *              2           switch off all channels with the same group number
 *              3           switch off the complete crate.
 *           iseg HV devices
 *              0           ignore the failure
 *              1           switch off this channel by ramp down the voltage
 *              2           switch off this channel by a emergencyOff
 *              3           switch off the whole board of the HV module by emergencyOff.
 *           The position of the bit fields in the integer value are:
 *              Bit 0, 1:   outputFailureMinSenseVoltage
 *              Bit 2, 3:   outputFailureMaxSenseVoltage
 *              Bit 4, 5:   outputFailureMaxTerminalVoltage
 *              Bit 6, 7:   outputFailureMaxCurrent
 *              Bit 8, 9:   outputFailureMaxTemperature
 *              Bit 10, 11: outputFailureMaxPower
 *              Bit 12, 13: outputFailureInhibit
 *              Bit 14, 15: outputFailureTimeout
 * @param session The handle returned by snmpOpen()
 * @param channel The channel (0...999) for which the behaviour should be set
 * @param value The 16 bit integer with bits set according the preceding table.
 * @return
 */
int setOutputSupervisionBehavior(HSNMP session, int channel, int value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetInt(session, &outputSupervisionBehavior[channel], value);
}

/**
 * @brief getOutputSupervisionMinSenseVoltage
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputSupervisionMinSenseVoltage(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputSupervisionMinSenseVoltage[channel]);
}

/**
 * @brief setOutputSupervisionMinSenseVoltage
 * @param session The handle returned by snmpOpen()
 * @param channel
 * @param value
 * @return
 */
double setOutputSupervisionMinSenseVoltage(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputSupervisionMinSenseVoltage[channel], value);
}

/**
 * @brief getOutputSupervisionMaxSenseVoltage
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputSupervisionMaxSenseVoltage(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputSupervisionMaxSenseVoltage[channel]);
}

/**
 * @brief setOutputSupervisionMaxSenseVoltage
 * @param session The handle returned by snmpOpen()
 * @param channel
 * @param value
 * @return
 */
double setOutputSupervisionMaxSenseVoltage(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputSupervisionMaxSenseVoltage[channel], value);
}

/**
 * @brief If the measured voltage at the power supply output terminals is above this value,
 * the power supply performs the function defined by setOutputSupervisionBehavior().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The maximum terminal voltage in Volt
 */
double getOutputSupervisionMaxTerminalVoltage(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputSupervisionMaxTerminalVoltage[channel]);
}

/**
 * @brief If the measured voltage at the power supply output terminals is above this value,
 * the power supply performs the function defined by setOutputSupervisionBehavior().
 * @param session The handle returned by snmpOpen()
 * @param channel the channel (0...999) to set the max. terminal voltage
 * @param value The maximum terminal voltage in Volt
 * @return
 */
double setOutputSupervisionMaxTerminalVoltage(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputSupervisionMaxTerminalVoltage[channel], value);
}

/**
 * @brief If the measured current is above this value, the power supply
 * performs the function defined by setOutputSupervisionBehavior().
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The maximum output current in Ampere
 */
double getOutputSupervisionMaxCurrent(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputSupervisionMaxCurrent[channel]);
}

/**
 * @brief If the measured current is above this value, the power supply
 * performs the function defined by setOutputSupervisionBehavior().
 * @param session The handle returned by snmpOpen()
 * @param channel The channel (0...999) to set the max. current
 * @param value The maximum current in Ampere
 * @return
 */
double setOutputSupervisionMaxCurrent(HSNMP session, int channel, double value)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetDouble(session, &outputSupervisionMaxCurrent[channel], value);
}

/**
 * @brief getOutputSupervisionMaxTemperature
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return The maximum temperature in degree Celsius
 */
int getOutputSupervisionMaxTemperature(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputSupervisionMaxTemperature[channel]);
}

/**
 * @brief getOutputConfigMaxSenseVoltage
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputConfigMaxSenseVoltage(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputConfigMaxSenseVoltage[channel]);
}

/**
 * @brief getOutputConfigMaxTerminalVoltage
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputConfigMaxTerminalVoltage(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputConfigMaxTerminalVoltage[channel]);
}

/**
 * @brief getOutputConfigMaxCurrent
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputConfigMaxCurrent(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputConfigMaxCurrent[channel]);
}

/**
 * @brief getOutputSupervisionMaxPower
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return
 */
double getOutputSupervisionMaxPower(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetDouble(session, &outputSupervisionMaxPower[channel]);
}

/**
 * @brief Returns the time span for the delayed trip function.
 * @param session The handle returned by snmpOpen()
 * @param channel The requested channel in the range of 0...999
 * @return the trip delay time (0...4000 ms)
 */
int getOutputTripTimeMaxCurrent(HSNMP session, int channel)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpGetInt(session, &outputTripTimeMaxCurrent[channel]);
}

/**
 * @brief Defines a span for the delayed trip function.
 * @param session The handle returned by snmpOpen()
 * @param channel The channel (0...999) for which to set the delayed trip
 * @param delay The trip delay time (0...4000 ms)
 * @return
 */
int setOutputTripTimeMaxCurrent(HSNMP session, int channel, int delay)
{
  if (channel < 0 || channel >= MaxChannelsPerCrate)
    return 0;

  return snmpSetInt(session, &outputTripTimeMaxCurrent[channel], delay);
}

// Sensor Information functions

int getSensorNumber(HSNMP session)
{
  return snmpGetInt(session, &sensorNumber);
}

int getSensorTemp(HSNMP session, int sensor)
{
  if (sensor < 0 || sensor > MaxSensors)
    return 0;

  return snmpGetInt(session, &sensorTemperature[sensor]);
}

int getSensorWarningTemperature(HSNMP session, int sensor)
{
  if (sensor < 0 || sensor > MaxSensors)
    return 0;

  return snmpGetInt(session, &sensorWarningThreshold[sensor]);
}

int setSensorWarningTemperature(HSNMP session, int sensor, int value)
{
  if (sensor < 0 || sensor > MaxSensors)
    return 0;

  return snmpSetInt(session, &sensorWarningThreshold[sensor], value);
}

int getSensorFailureTemperature(HSNMP session, int sensor)
{
  if (sensor < 0 || sensor > MaxSensors)
    return 0;

  return snmpGetInt(session, &sensorFailureThreshold[sensor]);
}

int setSensorFailureTemperature(HSNMP session, int sensor, int value)
{
  if (sensor < 0 || sensor > MaxSensors)
    return 0;

  return snmpSetInt(session, &sensorFailureThreshold[sensor], value);
}

// Power Supply specific Functions.

/**
 * @brief Returns the crates operating time in seconds.
 * @param session The handle returned by snmpOpen()
 * @return
 */
int getPsOperatingTime(HSNMP session)
{
  return snmpGetInt(session, &psOperatingTime);
}

double getPsAuxVoltage(HSNMP session, int auxIndex)
{
  if ((auxIndex < 0) || (auxIndex >= MaxPsAuxSupplies))
    return 0.0;

  return snmpGetDouble(session, &psAuxVoltage[auxIndex]);
}

double getPsAuxCurrent(HSNMP session, int auxIndex)
{
  if ((auxIndex < 0) || (auxIndex >= MaxPsAuxSupplies))
    return 0.0;

  return snmpGetDouble(session, &psAuxCurrent[auxIndex]);
}

// Fan Tray Functions

int getFanOperatingTime(HSNMP session)
{
  return snmpGetInt(session, &fanOperatingTime);
}

int getFanAirTemperature(HSNMP session)
{
  return snmpGetInt(session, &fanAirTemperature);
}

int getFanSwitchOffDelay(HSNMP session)
{
  return snmpGetInt(session, &fanSwitchOffDelay);
}

int setFanSwitchOffDelay(HSNMP session, int value)
{
  return snmpSetInt(session, &fanSwitchOffDelay, value);
}

/**
 * @brief Returns the MPODs fan rotation speed in revolutions per minute.
 * @param session The handle returned by snmpOpen()
 * @return
 */
int getFanNominalSpeed(HSNMP session)
{
  return snmpGetInt(session, &fanNominalSpeed);
}

/**
 * @brief Sets the MPODs fan rotation speed in revolutions per minute.
 * @param session The handle returned by snmpOpen()
 * @param value 1200..3600. 0 turns off the crates fans.
 * @return
 */
int setFanNominalSpeed(HSNMP session, int value)
{
  return snmpSetInt(session, &fanNominalSpeed, value);
}

int getFanNumberOfFans(HSNMP session)
{
  return snmpGetInt(session, &fanNumberOfFans);
}

int getFanSpeed(HSNMP session, int fan)
{
  if (fan < 0 || fan > MaxFans)
    return 0;

  return snmpGetInt(session, &fanSpeed[fan]);
}

/**
 * @brief Returns a pointer to the module description string.
 *
 * The pointer is valid until the next call of any string function.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @return A string with the following contents, separated by comma and space:
 * - The module vendor ("iseg" or "WIENER")
 * - The module type name
 * - The channel number
 * - The module serial number (optional)
 * - The module firmware release (optional)
 *
 * Example: "iseg, E24D1, 24, 715070, 5.14"
 */
char* getModuleDescription(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0;

  return snmpGetString(session, &moduleDescription[slot]);
}

/**
 * @brief Returns the measured value of the modules +24 Volt line.
 * @note This function is for iseg HV modules only.
 * @param session The handle returned by snmpOpen()
 * @param slot the modules slot position in the crate (0...9)
 * @return the measured +24 Volt line voltage in Volt.
 */
double getModuleSupply24(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpGetDouble(session, &moduleSupply[0][slot]);
}

/**
 * @brief Returns the measured value of the modules +5 Volt line.
 * @note This function is for iseg HV modules only.
 * @param session The handle returned by snmpOpen()
 * @param slot the modules slot position in the crate (0...9)
 * @return the measured +5 Volt line voltage in Volt.
 */
double getModuleSupply5(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpGetDouble(session, &moduleSupply[1][slot]);
}

/**
 * @brief Returns the measured value of one of the modules temperature sensors.
 *
 * @note This function is for iseg HV modules only.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @param index The temperature sensor index (0...3)
 * @note Most modules only have one temperature sensor at index 0.
 * @return
 */
double getModuleAuxTemperature(HSNMP session, int slot, int index)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  if (index < 0 || index >= MaxModuleAuxTemperatures)
    return 0.0;

  return snmpGetDouble(session, &moduleAuxiliaryMeasurementTemperature[index][slot]);
}

/**
 * @brief Returns the modules hardware voltage limit in percent.
 *
 * @note This function is for iseg HV modules only.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @return The modules hardware voltage limit in percent (2...102)
 */
double getModuleHardwareLimitVoltage(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpGetDouble(session, &moduleHardwareLimitVoltage[slot]);
}

/**
 * @brief Returns the modules hardware current limit in percent.
 *
 * @note This function is for iseg HV modules only.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @return The modules hardware current limit in percent (2...102)
 */
double getModuleHardwareLimitCurrent(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpGetDouble(session, &moduleHardwareLimitCurrent[slot]);
}

/**
 * @brief Returns the modules voltage ramp speed in percent.
 *
 * @note This function is for iseg HV modules only.
 * iseg modules have one common ramp speed for all channels.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @return The modules voltage ramp speed in percent
 */
double getModuleRampSpeedVoltage(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpGetDouble(session, &moduleRampSpeedVoltage[slot]);
}

/**
 * @brief Sets the modules voltage ramp speed in percent.
 *
 * @note This function is for iseg HV modules only.
 *
 * iseg modules have one common ramp speed for all channels.
 * @param session The handle returned by snmpOpen()
 * @param slot the modules slot position in the crate (0...9)
 * @param value The new voltage ramp speed in percent
 * @note For most modules, the range is 0.001...20 percent.
 * @return The new voltage ramp speed in percent
 */
double setModuleRampSpeedVoltage(HSNMP session, int slot, double value)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpSetDouble(session, &moduleRampSpeedVoltage[slot], value);
}

/**
 * @brief Returns the modules current ramp speed in percent.
 *
 * @note This function is for iseg HV modules only.
 *
 * iseg modules have one common ramp speed for all channels.
 * This item is only valid for modules with constant current regulation.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @return The modules current ramp speed in percent
 */
double getModuleRampSpeedCurrent(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpGetDouble(session, &moduleRampSpeedCurrent[slot]);
}

/**
 * @brief Sets the modules current ramp speed in percent.
 *
 * @note This function is for iseg HV modules only.
 *
 * iseg modules have one common ramp speed for all channels.
 * This item is only valid for modules with constant current regulation.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @param value The new current ramp speed in percent
 * @return The new current ramp speed in percent
 */
double setModuleRampSpeedCurrent(HSNMP session, int slot, double value)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0.0;

  return snmpSetDouble(session, &moduleRampSpeedCurrent[slot], value);
}

/**
 * @brief Returns the value of the module status register.
 *
 * @note This function is for iseg HV modules only.
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @return The module status register
 */
int getModuleStatus(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0;

  return snmpGetInt(session, &moduleStatus[slot]);
}

/**
 * @brief Returns the value of the module event status register.
 *
 * @note This function is for iseg HV modules only.
 * @param session The handle returned by snmpOpen()
 * @param slot the modules slot position in the crate (0...9)
 * @return The module event status register
 */
int getModuleEventStatus(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0;

  return snmpGetInt(session, &moduleEventStatus[slot]);
}

/**
 * @brief Clears all modules events in a specific slot.
 *
 * To clear all events in all iseg HV modules, use setHighVoltageGroupsSwitch()
 * with the parameter clearEvents(10).
 * @param session The handle returned by snmpOpen()
 * @param slot The modules slot position in the crate (0...9)
 * @return
 */
int setModuleDoClear(HSNMP session, int slot)
{
  if (slot < 0 || slot >= MaxSlotsPerCrate)
    return 0;

  return snmpSetInt(session, &moduleDoClear[slot], 1);
}

// The rest of the functions are utility functions that actually do the SNMP calls

static void logErrors(HSNMP session, struct snmp_pdu* response,
                      const SnmpObject* object, int status, const char* functionName)
{
  // FAILURE: print what went wrong!
  if (status == STAT_SUCCESS)
    snmp_log(LOG_ERR, "%s(%s): Error in packet. Reason: %s\n",
             functionName, object->desc, snmp_errstring(response->errstat));
  else
// ROK
    snmp_sess_perror("snmpget", snmp_sess_session(session));
  //printf("snmpget EROOR!!!\n");
}

static int getIntegerVariable(struct variable_list* vars)
{
  if (vars->type == ASN_BIT_STR || vars->type == ASN_OCTET_STR) {
    int value = 0;
    for (size_t i = 0; i < vars->val_len && i < sizeof(int); ++i)
      value |= (vars->val.bitstring[i] << (i * 8));
    return value;
  } if (vars->type == ASN_OPAQUE_FLOAT)
    return (int) * vars->val.floatVal;
  else if (vars->type == ASN_OPAQUE_DOUBLE)
    return (int) * vars->val.doubleVal;
  else if (vars->type == ASN_INTEGER)
    return *vars->val.integer;
  else if (vars->type == ASN_OCTET_STR)
    return *vars->val.integer;
  else if (vars->type == ASN_IPADDRESS)
    return *vars->val.integer;

  return 0;
}

static double getDoubleVariable(struct variable_list* vars)
{
  if (vars->type == ASN_OPAQUE_FLOAT)
    return *vars->val.floatVal;
  else if (vars->type == ASN_OPAQUE_DOUBLE)
    return *vars->val.doubleVal;
  else if (vars->type == ASN_INTEGER)
    return (double) * vars->val.integer;

  return 0.0;
}

static struct snmp_pdu* prepareSetRequestPdu(void)
{
  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_SET);
  pdu->community = (u_char*)strdup(m_writeCommunity);
  pdu->community_len = strlen(m_writeCommunity);

  return pdu;
}

static struct snmp_pdu* prepareGetRequestPdu()
{
  struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);

  return pdu;
}

static int snmpGetInt(HSNMP session, const SnmpObject* object)
{
  int value = 0;

  struct snmp_pdu* pdu = prepareGetRequestPdu();

  snmp_add_null_var(pdu, object->id, object->len);   // generate request data

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    value = getIntegerVariable(response->variables);
  } else {
    logErrors(session, response, object, status, "snmpGetInt");
    return -1;
  }

  snmp_free_pdu(response);
  return value;
}

static int snmpSetInt(HSNMP session, const SnmpObject* object, int value)
{
  struct snmp_pdu* pdu = prepareSetRequestPdu();

  if (snmp_oid_compare(object->id, object->len, ipStaticAddress.id, ipStaticAddress.len) == 0)
    snmp_pdu_add_variable(pdu, object->id, object->len, ASN_IPADDRESS, (u_char*)&value, sizeof(value));
  else
    snmp_pdu_add_variable(pdu, object->id, object->len, ASN_INTEGER, (u_char*)&value, sizeof(value));

  int result = value;
  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    result = getIntegerVariable(response->variables);
  } else {
    logErrors(session, response, object, status, "snmpSetInt");
    return -1;
  }

  snmp_free_pdu(response);
  return result;
}

static double snmpGetDouble(HSNMP session, const SnmpObject* object)
{
  double value = 0.0;

  struct snmp_pdu* pdu = prepareGetRequestPdu();

  snmp_add_null_var(pdu, object->id, object->len);   // generate request data

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    value = getDoubleVariable(response->variables);
  } else {
    logErrors(session, response, object, status, "snmpGetDouble");
    return -1;
  }

  snmp_free_pdu(response);
  return value;
}

static double snmpSetDouble(HSNMP session, const SnmpObject* object, double value)
{
  struct snmp_pdu* pdu = prepareSetRequestPdu();

  float v = (float)value;
  snmp_pdu_add_variable(pdu, object->id, object->len, ASN_OPAQUE_FLOAT, (u_char*)&v, sizeof(v));

  double result = v;

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    result = getDoubleVariable(response->variables);
  } else {
    logErrors(session, response, object, status, "snmpSetDouble");
    return -1;
  }

  snmp_free_pdu(response);
  return result;
}

static char* snmpGetString(HSNMP session, const SnmpObject* object)
{
  struct snmp_pdu* pdu = prepareGetRequestPdu();

  snmp_add_null_var(pdu, object->id, object->len);   // generate request data

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  memset(snmpStringBuffer, 0, sizeof(snmpStringBuffer));

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    struct variable_list* vars = response->variables;
    if (vars->type == ASN_OCTET_STR) {
      size_t len = sizeof(snmpStringBuffer) - 1;
      if (len > vars->val_len)
        len = vars->val_len;

      memcpy(snmpStringBuffer, vars->val.string, len);
      snmpStringBuffer[len] = 0;
    }
  } else {
    logErrors(session, response, object, status, "snmpGetString");
    return NULL;
  }

  snmp_free_pdu(response);
  return snmpStringBuffer;
}

static SnmpIntegerBuffer* snmpGetMultipleInteger(HSNMP session, const SnmpObject* objects, int size)
{
  struct snmp_pdu* pdu = prepareGetRequestPdu();

  if (size > MaxChannelsPerSlot)
    size = MaxChannelsPerSlot;

  memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));

  for (int i = 0; i < size; ++i)
    snmp_add_null_var(pdu, objects[i].id, objects[i].len);   // generate request data

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    struct variable_list* vars;
    for (vars = response->variables; vars; vars = vars->next_variable)
      snmpIntegerBuffer.value[snmpIntegerBuffer.size++] = getIntegerVariable(vars);
  } else {
    logErrors(session, response, &objects[0], status, "snmpGetMultipleInteger");
    return &snmpIntegerBuffer;
  }

  snmp_free_pdu(response);
  return &snmpIntegerBuffer;
}

static SnmpIntegerBuffer* snmpSetMultipleInteger(HSNMP session, const SnmpObject* objects, SnmpIntegerBuffer* values)
{
  struct snmp_pdu* pdu = prepareSetRequestPdu();

  int size = values->size;
  if (size > MaxChannelsPerSlot)
    size = MaxChannelsPerSlot;

  for (int i = 0; i < size; ++i) {
    int v = values->value[i];
    snmp_pdu_add_variable(pdu, objects[i].id, objects[i].len, ASN_INTEGER, (u_char*)&v, sizeof(v));
  }

  memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    struct variable_list* vars;
    for (vars = response->variables; vars; vars = vars->next_variable)
      snmpIntegerBuffer.value[snmpIntegerBuffer.size++] = getIntegerVariable(vars);
  } else {
    logErrors(session, response, &objects[0], status, "snmpSetMultipleInteger");
    return &snmpIntegerBuffer;
  }

  snmp_free_pdu(response);
  return &snmpIntegerBuffer;
}

static SnmpDoubleBuffer* snmpGetMultipleDouble(HSNMP session, const SnmpObject* objects, int size)
{
  struct snmp_pdu* pdu = prepareGetRequestPdu();

  if (size > MaxChannelsPerSlot)
    size = MaxChannelsPerSlot;

  memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));

  for (int i = 0; i < size; ++i)
    snmp_add_null_var(pdu, objects[i].id, objects[i].len);   // generate request data

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    struct variable_list* vars;
    for (vars = response->variables; vars; vars = vars->next_variable)
      snmpDoubleBuffer.value[snmpDoubleBuffer.size++] = getDoubleVariable(vars);
  } else {
    logErrors(session, response, &objects[0], status, "snmpGetMultipleDouble");
    return &snmpDoubleBuffer;
  }

  snmp_free_pdu(response);

  return &snmpDoubleBuffer;
}

static SnmpDoubleBuffer* snmpSetMultipleDouble(HSNMP session, const SnmpObject* objects, SnmpDoubleBuffer* values)
{
  struct snmp_pdu* pdu = prepareSetRequestPdu();

  int size = values->size;
  if (size > MaxChannelsPerSlot)
    size = MaxChannelsPerSlot;

  for (int i = 0; i < size; ++i) {
    float v = (float)values->value[i];
    snmp_pdu_add_variable(pdu, objects[i].id, objects[i].len, ASN_OPAQUE_FLOAT, (u_char*)&v, sizeof(v));
  }

  memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));

  struct snmp_pdu* response;
  int status = snmp_sess_synch_response(session, pdu, &response);

  if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    struct variable_list* vars;
    for (vars = response->variables; vars; vars = vars->next_variable)
      snmpDoubleBuffer.value[snmpDoubleBuffer.size++] = getDoubleVariable(vars);
  } else {
    logErrors(session, response, &objects[0], status, "snmpSetMultipleDouble");
    return &snmpDoubleBuffer;
  }

  snmp_free_pdu(response);
  return &snmpDoubleBuffer;
}

/**
 * @brief Returns an array with the outputStatus for a consecutive range of channels.
 *
 * @note This function is deprecated. Use getMultipleOutputStatuses() instead.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpIntegerBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpIntegerBuffer* getMultipleChannelStatuses(HSNMP session, int start, int size)
{
  return getMultipleOutputStatuses(session, start, size);
}

/**
 * @brief Returns an array with the outputStatus for a consecutive range of channels.
 * @since 1.1
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpIntegerBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpIntegerBuffer* getMultipleOutputStatuses(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));
    return &snmpIntegerBuffer;
  }

  return snmpGetMultipleInteger(session, &outputStatus[start], size);
}

/**
 * @brief Returns an array with the outputSwitches for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpIntegerBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpIntegerBuffer* getMultipleOutputSwitches(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));
    return &snmpIntegerBuffer;
  }

  return snmpGetMultipleInteger(session, &outputSwitch[start], size);
}

/**
 * @brief Sets the outputSwitch for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param values A pointer to SnmpIntegerBuffer with the list of outputSwitches.
 * @return
 */
SnmpIntegerBuffer* setMultipleOutputSwitches(HSNMP session, int start, SnmpIntegerBuffer* values)
{
  if (start < 0 || values->size < 0 || start + values->size > MaxChannelsPerCrate) {
    memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));
    return &snmpIntegerBuffer;
  }

  return snmpSetMultipleInteger(session, &outputSwitch[start], values);
}

/**
 * @brief Returns the actual outputVoltage for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpDoubleBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpDoubleBuffer* getMultipleOutputVoltages(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpGetMultipleDouble(session, &outputVoltage[start], size);
}

/**
 * @brief Sets the demanded outputVoltage for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param values A pointer to SnmpDoubleBuffer with the list of new outputVoltages
 * @return
 */
SnmpDoubleBuffer* setMultipleOutputVoltages(HSNMP session, int start, SnmpDoubleBuffer* values)
{
  if (start < 0 || values->size < 0 || start + values->size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpSetMultipleDouble(session, &outputVoltage[start], values);
}

/**
 * @brief Returns the measured terminal voltages for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpDoubleBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpDoubleBuffer* getMultipleMeasurementTerminalVoltages(HSNMP session, int start, int size)
{
  return getMultipleOutputMeasurementTerminalVoltages(session, start, size);
}

/**
 * @brief Returns an array with the measured terminal voltages for a consecutive range of channels.
 * @since 1.1
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpDoubleBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpDoubleBuffer* getMultipleOutputMeasurementTerminalVoltages(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpGetMultipleDouble(session, &outputMeasurementTerminalVoltage[start], size);
}

/**
 * @brief getMultipleOutputConfigMaxTerminalVoltages
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return
 */
SnmpDoubleBuffer* getMultipleOutputConfigMaxTerminalVoltages(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpGetMultipleDouble(session, &outputConfigMaxTerminalVoltage[start], size);
}

/**
 * @brief Returns an array the demanded output currents for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return
 */
SnmpDoubleBuffer* getMultipleOutputCurrents(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpGetMultipleDouble(session, &outputCurrent[start], size);
}

/**
 * @brief Sets the demanded output current for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param values A pointer to SnmpDoubleBuffer with a list of new output currents
 * @return
 */
SnmpDoubleBuffer* setMultipleOutputCurrents(HSNMP session, int start, SnmpDoubleBuffer* values)
{
  if (start < 0 || values->size < 0 || start + values->size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpSetMultipleDouble(session, &outputCurrent[start], values);
}

/**
 * @brief Returns an array with the measured currents for a consecutive range of channels.
 *
 * @note This function is deprecated. Use getMultipleOutputMeasurementCurrents() instead.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpDoubleBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpDoubleBuffer* getMultipleMeasurementCurrents(HSNMP session, int start, int size)
{
  return getMultipleOutputMeasurementCurrents(session, start, size);
}

/**
 * @brief Returns an array with the measured currents for a consecutive range of channels.
 * @since 1.1
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return A pointer to SnmpDoubleBuffer with the requested information.
 * @note This pointer is only valid until the next call of getMultiple...
 * or setMultiple... function.
 */
SnmpDoubleBuffer* getMultipleOutputMeasurementCurrents(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpGetMultipleDouble(session, &outputMeasurementCurrent[start], size);
}

/**
 * @brief Returns an array with the outputConfigMaxCurrent for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return
 */
SnmpDoubleBuffer* getMultipleOutputConfigMaxCurrents(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpDoubleBuffer, 0, sizeof(snmpDoubleBuffer));
    return &snmpDoubleBuffer;
  }

  return snmpGetMultipleDouble(session, &outputConfigMaxCurrent[start], size);
}

/**
 * @brief Returns an array with the outputTripTimeMaxCurrent for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return
 */
SnmpIntegerBuffer* getMultipleOutputTripTimeMaxCurrents(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));
    return &snmpIntegerBuffer;
  }

  return snmpGetMultipleInteger(session, &outputTripTimeMaxCurrent[start], size);
}

/**
 * @brief Sets the outputTripTimeMaxCurrent for a consecutive ranges of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param values
 * @return
 */
SnmpIntegerBuffer* setMultipleOutputTripTimeMaxCurrents(HSNMP session, int start, SnmpIntegerBuffer* values)
{
  if (start < 0 || values->size < 0 || start + values->size > MaxChannelsPerCrate) {
    memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));
    return &snmpIntegerBuffer;
  }

  return snmpSetMultipleInteger(session, &outputTripTimeMaxCurrent[start], values);
}

/**
 * @brief Returns an array with the outputSupervisionBehavior for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param size The number of requested channels.
 * @return
 */
SnmpIntegerBuffer* getMultipleOutputSupervisionBehaviors(HSNMP session, int start, int size)
{
  if (start < 0 || size < 0 || start + size > MaxChannelsPerCrate) {
    memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));
    return &snmpIntegerBuffer;
  }

  return snmpGetMultipleInteger(session, &outputSupervisionBehavior[start], size);
}

/**
 * @brief Sets the outputSupervisionBehavior for a consecutive range of channels.
 * @param session The handle returned by snmpOpen()
 * @param start The first channel (in the range of 0 to MaxArraySize).
 * 0 = slot 0, channel 0; 100 = slot 1, channel 0.
 * @param values The new outputSupervisionBehavior for the all channels starting with start.
 * @return
 */
SnmpIntegerBuffer* setMultipleOutputSupervisionBehaviors(HSNMP session, int start, SnmpIntegerBuffer* values)
{
  if (start < 0 || values->size < 0 || start + values->size > MaxChannelsPerCrate) {
    memset(&snmpIntegerBuffer, 0, sizeof(snmpIntegerBuffer));
    return &snmpIntegerBuffer;
  }

  return snmpSetMultipleInteger(session, &outputSupervisionBehavior[start], values);
}
//************************************************************************
