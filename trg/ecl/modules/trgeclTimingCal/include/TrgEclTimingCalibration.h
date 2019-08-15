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
#include "trg/ecl/dataobjects/TRGECLTrg.h"
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

    // Set TC position from TRGECLMap
    void Set_TCposition();

    // Solve matrix
    int  Solve_Matrix();

    // Fill data from
    int  ReadData();

    // Fill matrix and vector components
    int  FillMatrix();

    // Parameters initialization
    void InitParams();

    // Save time offset and chisq as a root file
    void Save_Result();

    // Calculate chisq based on time offset obtained from "Solve_Matrix" function
    void Calculate_Chisq();

    // Clear vector every event
    void TRGECLTimingCalClear();

    // Get previous time offsets (iteration mode)
    void GetTimeOffset();
  private:

    /***** Calibration *****/

    int fSimulation;     // Flag simulation 0 : real data  1 : simulation data
    int fCalType;        // Calibration type 0 : beam  1 : cosmic
    int f3Dbhabha_veto;  // Flag 3D bhabha veto 0 : not use  1 : use
    int fInclude_FWD;    // Flag include forward endcap 0 : exclude FWD  1 : include FWD
    int fInclude_BR;     // Flag include barrel  0 : exclude BR  1 : include BR
    int fInclude_BWD;    // Flag include backward endcap 0 : exclude BWD  1 : include BWD
    int fIteration;      // Flag iteration  0 : first calbiration  1 : iteration

    double cut_high_energy; // TC energy cut upper limit
    double cut_low_energy;  // TC energy cut lower limit

    int cut_ntc;  // The number of TC cut
    int TC_ref;   // Reference TC whose time offset set to be 0

    int nevt_selected = 0; // The number of selected events
    int nevt_read = 0;     // The number of read events
    double TCEnergyCalibrationConstant; // ADC to GeV energy conversion factor
    double cut_chisq; // TC chisq cut (iteration mode)


    std::string str_default_name = "TCTimeOffset.root"; // Default output root file name
    std::string str_ofilename;                          // Output root file name
    std::string str_timeoffset_fname;                   // Input time offset file name (iteration mode)

    // Trigger Cell
    std::vector<int>    TCId;           // TCID
    std::vector<double> TCEnergy;       // TC energy
    std::vector<double> TCTiming;       // TC time
    std::vector<TVector3> TCPosition;   // TC position from TRGECLMap (cosmic calibration)

    // Matrix
    TMatrixDSym _Matrix;                     // Marix component
    std::vector<std::vector<double>> Chisq1; // chisq component1
    std::vector<std::vector<double>> Chisq2; // chisq component2
    std::vector<std::vector<double>> Chisq3; // chisq component3

    std::vector<int> Nevent_TC;       // The number of uesed TC hit (TC by TC)
    std::vector<double> chisq_result; // Chisq values based on time offset obtained from "Solve_Matrix" function

    TVectorD _Vector;                     // Vector component
    std::vector<bool> b_Inc_TC;           // Flag TC included
    std::vector<bool> b_Exclude_TC;       // Flag TC excluded
    std::vector<double> tcal_result;      // Time offset calibration result
    std::vector<double> tcal_result_err;  // Time offset error

    int FlagMatrixSolved;    // Flag matrix solved (0 : not solved  1 : solved)

    // Time offset
    std::vector<double> TimeOffset; // Input time offset (iteration mode)

  };
}
#endif
