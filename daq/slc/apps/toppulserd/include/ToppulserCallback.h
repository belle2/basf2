#ifndef _Belle2_ToppulserCallback_h
#define _Belle2_ToppulserCallback_h

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/apps/toppulserd/PulseGeneratorInterface.h"

#include <string>

namespace Belle2 {

  /**
   *@brief struct to hold all relevant paramters which are read/written to pulser
   */
  struct PulserReadoutParameters {
    bool enabled;
    float dutyCycle;
    float frequency;
    float amplitude;
    std::string pulseShape;
  };

  /**
   *@brief TOP Pulse Generator Run Client Callback class
   *@author Tobias Weber
   */
  class ToppulserCallback : public RCCallback {

  public:
    ToppulserCallback();
    virtual ~ToppulserCallback() throw();

  public:
    /**
     *@brief initialize additional NSMV handlers and callback node
     */
    virtual void initialize(const DBObject& obj) throw(RCHandlerException);
    /**
     *@brief load configuration parameters from database and create pulser interface
     */
    virtual void configure(const DBObject& obj) throw(RCHandlerException);
    /**
     *@brief load all needed parameters to pulser and make it ready to go
     */
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void start(int expno, int runno) throw(RCHandlerException);
    virtual void stop() throw(RCHandlerException);
    virtual bool pause() throw(RCHandlerException);
    virtual bool resume(int subno) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);
    /**
     *@brief read current parameters from the pulser
     */
    virtual void monitor() throw(RCHandlerException);

    /*callbacks for nsmv variables*/
    void SetFrequency(float frequency);
    void OpenNewConnection(const std::string& newIpAddress);
    /*public variables to be changed by nsmv callbacks*/
    std::string ipAddress;
    float frequency;

  protected:

  private:

    /**
     *@brief Print pulser parameters to console
     */
    void PrintParameters(const PulserReadoutParameters& parameters);
    /**
     *@brief Load pulser parameters using the pulser interface
     */
    void LoadAllParametersToPulser();
    /**
     *@brief Read parameters from pulser
     *@param current values of parameters
     */
    void ReadParametersFromPulser(PulserReadoutParameters& parameters);
    /**
     *@brief update the current parameters in the nsm2 interface displayed to the GUI
     */
    void UpdateCallback();

    /**
     *@brief interface to pulse generator
     */
    PulseGeneratorInterface* pulseGeneratorInterface;

    /*private pulser variables
     *these are loaded from the database
     */
    bool configured;
    float dutyCycle;
    float amplitude;
    std::string pulseShape;
    int enabled;
  };
}

#endif
