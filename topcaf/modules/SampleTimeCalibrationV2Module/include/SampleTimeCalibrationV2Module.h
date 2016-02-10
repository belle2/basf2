#ifndef SampleTimeCalibrationV2Module_H
#define SampleTimeCalibrationV2Module_H

#include <framework/core/Module.h>

#include <map>
#include <string>
#include <TH1D.h>
#include <sstream>
#include <TFile.h>
#include <TProfile.h>
#include <TMinuit.h>

#include <topcaf/dataobjects/TOPCAFDigit.h>

namespace Belle2 {

  typedef std::vector< std::pair<double, double> > v_cell_dt_pair;

  class SampleTimeCalibrationV2Module : public Module {

  public:

    SampleTimeCalibrationV2Module();
    ~SampleTimeCalibrationV2Module();

//
    void initialize();
    void beginRun();
    void event();
    void terminate();



  private:

    //methods
    unsigned int GetWindowID(const TOPCAFDigit* in_digit);
    void FillWaveform(const TOPCAFDigit* in_digit);
    double CalibrateWaveform(const TOPCAFDigit* in_digit);

    //var
    std::string m_out_filename, m_in_filename;
    TFile* m_in_file, *m_out_file;
    std::map<topcaf_channel_id_t, TProfile*> m_sample2ped;

    int m_conditions;
    int m_mode;
    int m_writefile;

    std::string m_payload_tag, m_experiment, m_run;
    std::string m_initial_run, m_final_run;

    std::map<topcaf_channel_id_t, TProfile*> m_out_sample2time;
    std::map<topcaf_channel_id_t, TH1D*>  m_out_sample2time_corr;
    std::map<topcaf_channel_id_t, TH1D*>   m_in_chid2samplecalib, m_out_chid2samplecalib;
    std::map<topcaf_channel_id_t, TProfile*> m_in_sample2time;
    TH1D* m_waveform_h;
    TH1D* m_time_calib_tdc_h;
    TH1D* m_dt_h;
    double sum_max_time, num_max_time, m_tdc, m_cal_mint, m_cal_maxt;
    TH1D* m_residual_h, *m_corr_residual_h;
    TMinuit* m_Minuit;
    std::map<topcaf_channel_id_t, v_cell_dt_pair> m_ch2cell_dt;
    std::vector< std::pair<double, double> > v_oddevents, v_evenevents;

    int m_channel_samples;  // Can we get this from the data somehow?
    double m_time2tdc;
  };

  //  typedef std::vector<TOPCAFDigit*> TOPDigits;

}
#endif
