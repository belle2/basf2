#ifndef __BOARDSTACKSTATUS__
#define __BOARDSTACKSTATUS__

#include <string>
#include <vector>


/**
 *@brief Parameters like Temperatures etc. to be monitored
 */
struct BoardstackObservables {

  void SetNumberOfCarriers(const int nCarrier)
  {
    cPGPup.resize(nCarrier);
    cFirmwareVersion.resize(nCarrier);
    cSoftwareVersion.resize(nCarrier);
    cTriggerMask.resize(nCarrier);
    cDieTemperature.resize(nCarrier);
    cWallTemperature.resize(nCarrier);
    cASIC01Temperature.resize(nCarrier);
    cASIC23Temperature.resize(nCarrier);
    cARMState.resize(nCarrier);
  }

  //scrod observables
  int scrodID;
  int sFirmwareVersion, sSoftwareVersion;
  int sTriggerMask;
  float sDieTemperature, sWallTemperature, sHumidityTemperature;
  float sHumidity;
  float sVoltageRaw1, sVoltageRaw2, sVoltageRaw3;
  //float sVoltage25, sVoltage33, sVoltage25MIO, sVoltage18AuxIO;
  //float sVoltage18VCCO, sVoltage25VCCO, sVoltage10GTX, sVoltage12GTX;
  std::string sARMState;

  //carrier parameters
  std::vector<int> cPGPup;
  std::vector<int> cFirmwareVersion, cSoftwareVersion;
  std::vector<int> cTriggerMask;
  std::vector<float> cDieTemperature, cWallTemperature, cASIC01Temperature, cASIC23Temperature;
  std::vector<std::string> cARMState;
};

namespace Belle2 {
  class HSLB;
  class RCCallback;
}

/**
 *@brief class with functions to monitor board stack status with nsm2 or in terminal
 */
class BoardStackStatus {
public:
  BoardStackStatus();
  BoardStackStatus(const unsigned nCarriers);
  ~BoardStackStatus() {};

  void SetNumberOfCarriers(const unsigned nCarriers);
  const BoardstackObservables GetBoardstackObservables() {return m_boardstackObservables;}

  void InitNSMCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  void UpdateNSMCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  void PrintBoardstackStatus(Belle2::HSLB& hslb);

private:

  void InitNSMCallbacksSCROD(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  void InitNSMCallbacksCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback,
                               const unsigned carrier);

  void UpdateNSMCallbacksSCROD(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  void UpdateNSMCallbacksCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback,
                                 const unsigned carrier);

  bool IsBoardstackGood();

  void PrintSCRODStatus();
  void PrintCarrierStatus(const unsigned carrier);

  void ReadSCRODObervables(Belle2::HSLB& hslb);
  void ReadCarrierObservables(Belle2::HSLB& hslb, const unsigned carrier);
  void UpdateARMLiveCounters(Belle2::HSLB& hslb, Belle2::RCCallback& callback);

  BoardstackObservables m_boardstackObservables;
  unsigned m_nCarriers;

};

#endif
