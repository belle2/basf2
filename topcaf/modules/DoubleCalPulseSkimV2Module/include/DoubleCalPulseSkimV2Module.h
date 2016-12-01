#ifndef DoubleCalPulseSkimV2Module_H
#define DoubleCalPulseSkimV2Module_H

#include <map>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"
#include <fstream>
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {
  class DoubleCalPulseSkimV2Module : public Module {
  public:

    DoubleCalPulseSkimV2Module();
    ~DoubleCalPulseSkimV2Module();

    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    int m_cal_ch;
    int m_runno;
    double m_tmin, m_tmax;
    double m_wmin, m_wmax;
    double m_adcmin, m_adcmax;
    int m_skim = 1; //steerig skim
    int h_trigger;//event number instead of frames
    std::string m_out_filename;//text file containing outputs
    std::ofstream ofs;

//! Save root file
    TFile* m_out_file;
    TTree* ttree1;
    TTree* ttree2;
    Int_t runno;
    Int_t h_events;
    Int_t evtnum;
    Int_t h_asic_id;
    Int_t   h_asic_chan;
    Int_t   h_win;
    Float_t h_tdc1;
    Float_t h_adc1;
    Float_t h_tdc2;
    Float_t h_adc2;

    int calch;
    int trig;
    int bs2;
    int ca2;
    int as2;
    int ch2;
    int win2;
    int flag;
    float tdc2;
    float adc2;
    float width2;
    float chg_int;
  };

}
#endif
