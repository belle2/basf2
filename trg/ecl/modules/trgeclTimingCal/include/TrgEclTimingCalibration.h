#ifndef TRGECLTCALIBRATION_h
#define TRGECLTCALIBRATION_h


#include <iostream>
#include <fstream>
#include <vector>

#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompLU.h"
#include "TVector3.h"
#include "TFile.h"
#include "TTree.h"

#include <framework/core/Module.h>
#include <trg/ecl/TrgEclMapping.h>

#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerEvtStore.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {
  class TRGECLTimingCalModule : public Module {

  public:
    TRGECLTimingCalModule();
    virtual ~TRGECLTimingCalModule();

  public:
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    void Set_TCposition();
    int  Solve_Matrix();
    void ReadData();
    void FillMatrix();
    void InitParams();
    void Save_Result();
    void Calculate_Chisq();
    void TRGECLTimingCalClear();
    void GetTimeOffset(); // iteration mode
  private:

    // Calibration
    int fSimulation;
    int fCalType;
    int f3Dbhabha_veto;

    int fInclude_FWD;
    int fInclude_BR;
    int fInclude_BWD;
    int fCalSameTheta;
    int fIteration;

    double cut_high_energy;
    double cut_low_energy;

    int cut_ntc;
    int TC_ref;
    int ThetaID_CalSameTheta;

    int nevt_selected;
    int nevt_read;
    double TCEnergyCalibrationConstant;
    double cut_chisq;


    std::string str_default_name = "TCTimeOffset.root";
    std::string str_ofilename;
    std::string str_timeoffset_fname;

    // Trigger Cell
    std::vector<int>    TCId;
    std::vector<double> TCEnergy;
    std::vector<double> TCTiming;

    std::vector<TVector3> TCPosition;

    // Cluster
    std::vector<int>    Cluster_maxtcid;
    std::vector<double> Cluster_energy;
    double ClusterEnergy;
    int ICN;

    // Matrix
    TMatrixDSym _Matrix;
    //std::vector<std::vector<double>> _Matrix;

    std::vector<std::vector<double>> Chisq1;
    std::vector<std::vector<double>> Chisq2;
    std::vector<std::vector<double>> Chisq3;

    std::vector<int> Nevent_TC;
    std::vector<double> chisq_result;

    TVectorD _Vector;
    //std::vector<double> _Vector;
    std::vector<bool> b_Inc_TC;
    std::vector<double> tcal_result;
    std::vector<double> tcal_result_err;
    std::vector<double> TOF_IP2TC;
    std::vector<bool> b_Exclude_TC;
    int FlagMatrixSolved;

    // Time offset

    std::vector<double> TimeOffset;

  };
}
#endif
