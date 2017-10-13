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
  int sFEMode;
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

  /**
   *@brief Set number of carriers connected to board stacks
   */
  void SetNumberOfCarriers(const unsigned nCarriers);
  /**
   *@brief returns monitored boardstack observables
   */
  const BoardstackObservables GetBoardstackObservables() {return m_boardstackObservables;}

  /**
   *@brief Initialize the NSM callbacks
   *@param hslb connected to boardstack
   *@param run control callback
   */
  void InitNSMCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  /**
   *@brief Update NSM callbacks
   *@param hslb connected to boardstack
   *@param run control callback
   *@return true of board stack is in a good state
   */
  bool UpdateNSMCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  /**
   *@brief print board stack status to terminal
   *@param hslb connected to boardstack
   */
  void PrintBoardstackStatus(Belle2::HSLB& hslb);

private:

  /**
   *@brief Initialize the scrod NSM callbacks
   *@param hslb connected to boardstack
   *@param run control callback
   */
  void InitNSMCallbacksSCROD(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  /**
   *@brief Initialize the carrier NSM callbacks
   *@param hslb connected to boardstack
   *@param run control callback
   *@param carrier ID
   */
  void InitNSMCallbacksCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback,
                               const unsigned carrier);

  /**
   *@brief Update scrod NSM callbacks
   *@param hslb connected to boardstack
   *@param run control callback
   */
  void UpdateNSMCallbacksSCROD(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  /**
   *@brief Update scrod NSM callbacks
   *@param hslb connected to boardstack
   *@param run control callback
   *@param carrier ID
   */
  void UpdateNSMCallbacksCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback,
                                 const unsigned carrier);

  /**
   *@brief compute overall board stack status
   *@return true if overall board stack status is fine (maybe change to int for severity level?)
   */
  bool IsBoardstackGood();

  /**
   *@brief print SCROD status to terminal
   */
  void PrintSCRODStatus();
  /**
   *@brief print carrier status to terminal
   *@param carrier ID
   */
  void PrintCarrierStatus(const unsigned carrier);

  /**
   *@brief read the current values of the SCROD observables via belle2 link
   *@param hslb connected to board stack
   */
  void ReadSCRODObervables(Belle2::HSLB& hslb);
  /**
   *@brief read the current values of the carrier observables via belle2 link
   *@param hslb connected to board stack
   *@param carrier ID
   */
  void ReadCarrierObservables(Belle2::HSLB& hslb, const unsigned carrier);
  /**
   *@brief update the ARM core status by reading the live counters
   *@param hslb connected to board stack
   *@param run control callback
   */
  void UpdateARMLiveCounters(Belle2::HSLB& hslb, Belle2::RCCallback& callback);

  BoardstackObservables m_boardstackObservables;/**<values monitored on board stack>*/
  unsigned m_nCarriers;/**<number of carriers connected to board stack>*/

};

#endif
