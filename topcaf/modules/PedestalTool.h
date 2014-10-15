#ifndef PedestalTool_H
#define PedestalTool_H
#include <map>
#include <string>
#include <TH1D.h>
#include <sstream>
#include <TFile.h>
#include <TProfile.h>

#include "EventWaveformPacket.h"
#include "RawEvent.h"
#include "iTopUtils.h"

class PedestalTool {

public:

  PedestalTool();
  ~PedestalTool();

  //////////////////////////
  // To store information //
  //////////////////////////

  //Invokes the output pedestal file
  void CreatePedestalFile(const char* output_file_name);

  //Method used to record adc valies from waveform
  void FillWavePacket(const EventWaveformPacket* wp);

  //Set output request in case you defined your own file
  void SetDBOutputRequest(void);

  /////////////////////////////
  // To retrieve information //
  /////////////////////////////

  //Load the pedestal file
  int LoadPedestalFile(const char* input_file_name);

  // Load pedestal info from DB
  int LoadPedestalDB(std::string global_tag, Int_t run_number);

  //Applies pedestal correction at wave level
  EventWaveformPacket* PedCorrect(const EventWaveformPacket* wp);

  //Get the name and/or version of this algorithm
  std::string GetAlgName() {return "Pedestal";};
  std::string GetVersion() {return "1_0_0_1";};
  std::string GetPayloadTag() {return m_payload_tag;};
  std::string GetSubsystemTag() {return "iTOP";};

  Int_t GetInitialRun() {return m_initial_run;};
  Int_t GetFinalRun() {return m_final_run;};

  void SetPayloadTag(std::string newtag) {m_payload_tag = newtag;};
  void SetFinalRun(Int_t final_run) {m_final_run = final_run;};
  void SetInitialRun(Int_t initial_run) {m_initial_run = initial_run;};


  RawEvent* PedCorrect(const RawEvent* raw_event);

private:

  //method -- should be move to a util class (iTopUtils)
  unsigned int GetWindowID(const EventWaveformPacket* wp);

  //var
  TFile* m_in_ped_file, *m_out_ped_file;
  std::map<unsigned int, TProfile*> m_in_sample2ped, m_out_sample2ped;

  Bool_t m_dbout_request;

  std::string m_payload_tag;
  Int_t m_initial_run, m_final_run;

};
#endif
