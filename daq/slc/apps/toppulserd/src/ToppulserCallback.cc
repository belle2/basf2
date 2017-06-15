#include "daq/slc/apps/toppulserd/ToppulserCallback.h"

#include <daq/slc/system/LogFile.h>

#include <iostream>

using namespace Belle2;
using namespace std;

class PulserFrequencyHandler : public NSMVHandlerFloat {
public:
  PulserFrequencyHandler(const std::string& name, ToppulserCallback& callback)
    : NSMVHandlerFloat(name, true, true), m_callback(callback) {}

  bool handleSetFloat(float val)
  {
    m_callback.SetFrequency(val);
    NSMVHandlerFloat::handleSetFloat(val);
    m_callback.frequency = val;
    LogFile::info("new frequency set %f", val);
    return true;
  }

private:
  ToppulserCallback& m_callback;
};

class PulserIPAddressHandler : public NSMVHandlerText {
public:
  PulserIPAddressHandler(const std::string& name, ToppulserCallback& callback)
    : NSMVHandlerText(name, true, true), m_callback(callback) {}

  bool handleSetText(const std::string& val)
  {
    LogFile::info("new ip-address for pulser : %s", val.c_str());
    m_callback.OpenNewConnection(val);
    m_callback.ipAddress = val;
    return true;
  }

private:
  ToppulserCallback& m_callback;
};

ToppulserCallback::ToppulserCallback()
{
  m_showall = true;
  pulseGeneratorInterface = NULL;
}

ToppulserCallback::~ToppulserCallback() throw()
{
  if (pulseGeneratorInterface) delete pulseGeneratorInterface;
}

/*
   initialize() : Initialize with parameters from configuration database.
                  Called at the beginings of NSM node.
   obj          : Contains parameters in configuration from the database.
*/
void ToppulserCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  //just disable channel output
  enabled = 0;
  configured = false;
  add(new NSMVHandlerInt("configured", true, false, 0));
  add(new PulserFrequencyHandler("frequency_nsmv", *this));
  add(new PulserIPAddressHandler("ipaddress_nsmv", *this));
  UpdateCallback();
}

/*
   configure() : Read parameters from configuration database.
                 Called by RC_CONFIGURE at NOTREADY.
   obj         : Contains parameters in configuration from the database.
*/
void ToppulserCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  //get values from database
  bool error = false;

  if (obj.hasObject("value")) {
    const DBObject& o_value(obj("value"));
    if (o_value.hasValue("dutycycle")) {
      dutyCycle = o_value.getFloat("dutycycle");
    } else {
      LogFile::debug("error getting dutycycle from database");
      error = true;
      dutyCycle = 50;
    }
    if (o_value.hasValue("frequency")) {
      frequency = o_value.getFloat("frequency");
    } else {
      LogFile::debug("error getting frequency from database");
      error = true;
      frequency = 100;
    }
    if (o_value.hasValue("amplitude")) {
      amplitude = o_value.getFloat("amplitude");
    } else {
      LogFile::debug("error getting amplitude from database");
      error = true;
      amplitude = 5.;
    }
    if (o_value.hasValue("enabled")) {
      enabled = o_value.getBool("enabled");
    } else {
      LogFile::debug("error getting channel enable from database");
      error = true;
      enabled = 0;
    }
    if (o_value.hasText("ipaddress")) {
      ipAddress = o_value.getText("ipaddress");
    } else {
      LogFile::debug("error getting IP Address");
      error = true;
      ipAddress = "192.168.30.222";
    }
    if (o_value.hasText("pulseshape")) {
      pulseShape = o_value.getText("pulseshape");
    } else {
      LogFile::debug("error getting pulse shape");
      error = true;
      pulseShape = "PULS";
    }
  } else {
    LogFile::debug("did not find value in database");
    error = true;
  }
  if (!error) {
    configured = true;
  }
  //set values from database to ip address and frequency
  set("ipaddress_nsmv", ipAddress);
  set("frequency_nsmv", frequency);

  //initialize the pulse generator interface
  pulseGeneratorInterface = new PulseGeneratorInterface();
  //pulseGeneratorInterface->EnableDebugMode(true);
  int status = pulseGeneratorInterface->OpenConnection(ipAddress);
  if (status != 0) {
    LogFile::debug("Error connecting to pulse generator at %s", ipAddress.c_str());
    return;
  }
  //report to GUI that configuration is done
  set("configured", configured);
  UpdateCallback();
}

/*
   monitor() : Check status at each end of wainting time limit.
*/
void ToppulserCallback::monitor() throw(RCHandlerException)
{
  UpdateCallback();
}

/*
   load() : Perform parameter download etc to READY.
   obj    : Contains configuration read by the last initailize or configure.
            Values in obj might be modified by nsmvset.
*/
void ToppulserCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  //load parameters from configure into device
  LoadAllParametersToPulser();
  LogFile::debug("Load done");
}

/*
   start() : Start triggers to RUNNING
   expno   : Experiment number from RC parent
   runno   : Run number from RC parent
*/
void ToppulserCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
  //pulser output on
  enabled = 1;
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->SetChannelOutput(true);
  }
  UpdateCallback();
  LogFile::debug("Start done");

}

/*
   stop() : End run by stopping triggers to READY
*/
void ToppulserCallback::stop() throw(RCHandlerException)
{
  LogFile::debug("Stop done");
  //pulser output off
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->SetChannelOutput(false);
  }
  UpdateCallback();
  enabled = 0;
}

/*
   resume() : Restart triggers back to RUNNING
   subno    : Sub run number for the resumed run
   returns true on success or false on failed
*/
bool ToppulserCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("sub run # = %03d", subno);
  //pulser output on
  enabled = 1;
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->SetChannelOutput(true);
  }
  UpdateCallback();
  LogFile::debug("Resume done");
  return true;
}

/*
   pause() : Suspend triggers to PAUSED
   returns true on success or false on failed
*/
bool ToppulserCallback::pause() throw(RCHandlerException)
{
  //pulser output off
  enabled = 0;
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->SetChannelOutput(false);
  }
  UpdateCallback();
  LogFile::debug("Pause done");
  return true;
}

/*
   recover() : Recover errors to be back to READY
*/
void ToppulserCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  // abort to be NOTREADY
  abort();
  // load to be READY
  load(obj);
  LogFile::debug("Recover done");
}

/*
   abort() : Discard all configuration to back to NOTREADY
*/
void ToppulserCallback::abort() throw(RCHandlerException)
{
  //pulser off
  enabled = 0;
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->SetChannelOutput(false);
  }
  UpdateCallback();
  LogFile::debug("Abort done");
}

void ToppulserCallback::SetFrequency(float frequency)
{
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->SetFrequency(frequency);
  }
}

void ToppulserCallback::OpenNewConnection(const std::string& newIpAddress)
{
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->CloseConnection();
    pulseGeneratorInterface->OpenConnection(newIpAddress);
  }
}

void ToppulserCallback::PrintParameters(const PulserReadoutParameters& parameters)
{
  cout << "Current Operation Parameters in TOP Pulser Callback" << endl;
  cout << "dutycycle: " << parameters.dutyCycle << endl;
  cout << "frequency: " << parameters.frequency << endl;
  cout << "amplitude: " << parameters.amplitude << endl;
  cout << "pulseShape: " << parameters.pulseShape << endl;
  cout << "channel on: " << parameters.enabled << endl;
}

void ToppulserCallback::LoadAllParametersToPulser()
{
  if (pulseGeneratorInterface != NULL) {
    pulseGeneratorInterface->SetFrequency(frequency);
    pulseGeneratorInterface->SetDutyCycle(dutyCycle);
    pulseGeneratorInterface->SetAmplitude(amplitude);
    pulseGeneratorInterface->SetPulseShape(pulseShape);
  }
  UpdateCallback();
}

void ToppulserCallback::ReadParametersFromPulser(PulserReadoutParameters& parameter)
{
  if (pulseGeneratorInterface != NULL) {
    parameter.enabled = pulseGeneratorInterface->GetChannelIsOn();
    parameter.dutyCycle = pulseGeneratorInterface->GetDutyCycle();
    parameter.frequency = pulseGeneratorInterface->GetFrequency();
    parameter.amplitude = pulseGeneratorInterface->GetAmplitude();
    parameter.pulseShape = pulseGeneratorInterface->GetPulseShape();
  }
}

void ToppulserCallback::UpdateCallback()
{
  PulserReadoutParameters currentParameters;
  ReadParametersFromPulser(currentParameters);
  //PrintParameters(currentParameters);
  set("value.dutycycle", currentParameters.dutyCycle);
  set("frequency_nsmv", currentParameters.frequency);
  set("value.ampltiude", currentParameters.amplitude);
  set("value.enabled", currentParameters.enabled);
  set("value.pulseshape", currentParameters.pulseShape);
  set("ipaddress_nsmv", ipAddress);
  set("configured", configured);
}
