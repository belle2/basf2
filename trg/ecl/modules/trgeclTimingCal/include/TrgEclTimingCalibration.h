#ifndef TRGECLTCALIBRATION_h
#define TRGECLTCALIBRATION_h


#include <iostream>
#include <fstream>
#include <vector>

#include "TMatrixD.h"
#include "TVectorD.h"
#include "TVector3.h"

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
  //! Class ECL Trigger Timiing Calibration Module
  class TRGECLTimingCalModule : public Module {

  public:
    //! Constructor
    TRGECLTimingCalModule();
    //! Destructor
    virtual ~TRGECLTimingCalModule();

  public:
    //! initialize function
    virtual void initialize() override;
    //! Begin Run function
    virtual void beginRun() override;
    //! Event function
    virtual void event() override;
    //! End Run function
    virtual void endRun() override;
    //! Terminate function
    virtual void terminate() override;

    //! Set TC position from TRGECLMap
    void Set_TCposition();

    //! Solve matrix
    int  Solve_Matrix();

    //! Fill data from
    int  ReadData();

    //! Fill matrix and vector components
    int  FillMatrix();

    //! Parameters initialization
    void InitParams();

    //! Save time offset and chisq as a root file
    void Save_Result();

    //! Calculate chisq based on time offset obtained from "Solve_Matrix" function
    void Calculate_Chisq();

    //! Clear vector every event
    void TRGECLTimingCalClear();

    //! Get previous time offsets (iteration mode)
    void GetTimeOffset();
  private:

    /***** Calibration *****/
    //! Flag simulation 0 : real data  1 : simulation data
    int fSimulation;
    //! Calibration type 0 : beam  1 : cosmic
    int fCalType;
    //! Flag 3D bhabha veto 0 : not use  1 : use
    int f3Dbhabha_veto;
    //! Flag include forward endcap 0 : exclude FWD  1 : include FWD
    int fInclude_FWD;
    //! Flag include barrel  0 : exclude BR  1 : include BR
    int fInclude_BR;
    //! Flag include backward endcap 0 : exclude BWD  1 : include BWD
    int fInclude_BWD;
    //! Flag iteration  0 : first calbiration  1 : iteration
    int fIteration;

    //! TC energy cut upper limit
    double cut_high_energy;
    //! TC energy cut lower limit
    double cut_low_energy;
    //! The number of TC cut
    int cut_ntc;
    //! Reference TC whose time offset set to be 0
    int TC_ref;

    //! The number of selected events
    int nevt_selected = 0;
    //! The number of read events
    int nevt_read = 0;
    //! ADC to GeV energy conversion factor
    double TCEnergyCalibrationConstant;
    //! TC chisq cut (iteration mode)
    double cut_chisq;

    //! Default output root file name
    std::string str_default_name = "TCTimeOffset.root";
    //! Output root file name
    std::string str_ofilename;
    //! Input time offset file name (iteration mode)
    std::string str_timeoffset_fname;

    //! Trigger Cell
    //! TCID
    std::vector<int>    TCId;
    //! TC energy
    std::vector<double> TCEnergy;
    //! TC time
    std::vector<double> TCTiming;
    //! TC position from TRGECLMap (cosmic calibration)
    std::vector<TVector3> TCPosition;

    //! Matrix
    //! Marix component
    TMatrixDSym _Matrix;
    //! chisq component1
    std::vector<std::vector<double>> Chisq1;
    //! chisq component2
    std::vector<std::vector<double>> Chisq2;
    //! chisq component3
    std::vector<std::vector<double>> Chisq3;

    //! The number of uesed TC hit (TC by TC)
    std::vector<int> Nevent_TC;
    //! Chisq values based on time offset obtained from "Solve_Matrix" function
    std::vector<double> chisq_result;
    //! Vector component
    TVectorD _Vector;
    //! Flag TC included
    std::vector<bool> b_Inc_TC;
    //! Flag TC excluded
    std::vector<bool> b_Exclude_TC;
    //! Time offset calibration result
    std::vector<double> tcal_result;
    //! Time offset error
    std::vector<double> tcal_result_err;

    //! Flag matrix solved (0 : not solved  1 : solved)
    int FlagMatrixSolved = 0;

    //! Time offset
    //! Input time offset (iteration mode)
    std::vector<double> TimeOffset;

  };
}
#endif
