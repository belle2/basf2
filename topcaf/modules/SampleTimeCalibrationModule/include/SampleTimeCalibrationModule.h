#ifndef SampleTimeCalibrationModule_H
#define SampleTimeCalibrationModule_H

#include <framework/core/Module.h>

#include <map>
#include <string>
#include <TH1D.h>
#include <sstream>
#include <TFile.h>
#include <TProfile.h>
#include <TMinuit.h>

#include <topcaf/dataobjects/EventWaveformPacket.h>

#include <top/dataobjects/TOPDigit.h>

namespace Belle2 {

  typedef std::vector< std::pair<double, double> > v_cell_dt_pair;

  class SampleTimeCalibrationModule : public Module {

  public:

    SampleTimeCalibrationModule();
    ~SampleTimeCalibrationModule();

//
    void initialize();
    void beginRun();
    void event();
    void terminate();


    //Get the name and/or version of this algorithm
    std::string GetVersion() {return "1.0.0.1";};
    std::string GetPayloadTag() {return m_payload_tag;};
    std::string GetPayloadType() {return "calibration";};
    std::string GetExperiment() {return m_experiment;};

    std::string GetInitialRun() {return m_initial_run;};
    std::string GetFinalRun() {return m_final_run;};


  private:

    //methods
    unsigned int GetWindowID(const EventWaveformPacket* wp);
    void FillWaveform(const EventWaveformPacket* in_wp);
    double CalibrateWaveform(const EventWaveformPacket* in_wp);

    //var
    std::string m_out_filename, m_in_filename;
    TFile* m_in_file, *m_out_file;
    std::map<unsigned int, TProfile*> m_sample2ped;

    int m_conditions;
    int m_mode;
    int m_writefile;

    std::string m_payload_tag, m_experiment, m_run;
    std::string m_initial_run, m_final_run;

    std::map<unsigned int, TProfile*> m_out_sample2time;
    std::map<unsigned int, TH1D*>  m_out_sample2time_corr;
    std::map<unsigned int, TH1D*>   m_in_chid2samplecalib, m_out_chid2samplecalib;
    std::map<unsigned int, TProfile*> m_in_sample2time;
    TH1D* m_waveform_h;
    TH1D* m_time_calib_tdc_h;
    TH1D* m_dt_h;
    double sum_max_time, num_max_time, m_tdc;
    TH1D* m_residual_h, *m_corr_residual_h;
    TMinuit* m_Minuit;
    std::map<unsigned int, v_cell_dt_pair> m_ch2cell_dt;
    std::vector< std::pair<double, double> > v_oddevents, v_evenevents;

    int m_channel_samples;  // Can we get this from the data somehow?

  };

  typedef std::vector<TOPDigit*> TOPDigits;

}
#endif
