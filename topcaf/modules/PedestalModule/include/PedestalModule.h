#ifndef PedestalTool_H
#define PedestalTool_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <map>
#include <string>
#include <TH1D.h>
#include <sstream>
#include <TFile.h>
#include <TProfile.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/topFileMetaData.h>

namespace Belle2 {
  class PedestalModule : public Module {
  public:

    PedestalModule();
    ~PedestalModule();

    void initialize();
    void beginRun();
    void event();
    void terminate();

    //Get the name and/or version of this algorithm
    std::string GetAlgName() {return getName();};
    std::string GetVersion() {return "1.0.0.1";};
    std::string GetPayloadTag() {return m_payload_tag;};
    std::string GetPayloadType() {return "calibration";};
    std::string GetSubsystemTag() {return "top";};
    std::string GetExperiment() {return m_experiment;};

    std::string GetInitialRun() {return m_initial_run;};
    std::string GetFinalRun() {return m_final_run;};

    //void SetPayloadTag(std::string newtag){m_payload_tag=newtag;};
    //void SetFinalRun(Int_t final_run){m_final_run=final_run;};
    //void SetInitialRun(Int_t initial_run){m_initial_run=initial_run;};
  private:

    //var
    std::string m_out_ped_filename, m_in_ped_filename;
    TFile* m_in_ped_file, *m_out_ped_file;
    std::map<topcaf_channel_id_t, TProfile*> m_sample2ped;

    Int_t m_conditions;
    Int_t m_mode;
    Int_t m_writefile;

    std::string m_payload_tag, m_experiment, m_run;
    std::string m_initial_run, m_final_run;
    StoreObjPtr<topFileMetaData> m_metadata_ptr;
    StoreObjPtr<EventHeaderPacket> m_evthead_ptr;
    StoreArray<EventWaveformPacket> m_evtwaves_ptr;
  };
}
#endif
