//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclFAM.h
// Section  : TRG ECL
// Owner    : InSu Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------
#ifndef TRGECLFAMFIT_H
#define TRGECLFAMFIT_H

#include <TObject.h>
#include "trg/ecl/TrgEclMapping.h"
#include <trg/ecl/TrgEclDataBase.h>

namespace Belle2 {

  /*! FAM module   */
  class TrgEclFAMFit : public TObject {

  public:

    /** Constructor */
    TrgEclFAMFit();

    /** Destructor */
    virtual ~TrgEclFAMFit();

    /** setup fam module  */
    void setup(int);
    /** save fitting result into tables */
    void save(int);
    /** function for fitting  */
    void FAMFit01(std::vector<std::vector<double>>, std::vector<std::vector<double>>);
    /** function for backup 1  */
    void FAMFit02(std::vector<std::vector<double>>, std::vector<std::vector<double>>);
    /** function for backup2 */
    void FAMFit03(std::vector<std::vector<double>>, std::vector<std::vector<double>>);
    /** Set Beam Background Tag flag */
    void SetBeamBkgTagFlag(int beambkgtagflag) {_BeamBkgTag = beambkgtagflag;}
    /** Set flag for saving analysis table*/
    void SetAnaTagFlag(int anatagflag) {_AnaTag = anatagflag;}
    /** Set Threshold */
    void SetThreshold(const std::vector<int>& threshold)
    {
      Threshold = threshold;
    };
    /** Set Beam Background Tag */
    void SetBeamBkgTag();

    //! Get TC Energy
    std::vector<std::vector<double>>  GetFitEnergy() {return TCFitEnergy;}
    //! Get TC Timing
    std::vector<std::vector<double>>  GetFitTiming() {return TCFitTiming;}
    //! Get Background Tag of TC Hit
    std::vector<std::vector<int>>  GetBeamBkgTag() {return BeamBkgTag;}



  private:
    /** fit energy */
    std::vector<std::vector<double>>  TCFitEnergy;
    /** fit timing */
    std::vector<std::vector<double>>  TCFitTiming;
    /** fit timing */
    std::vector<std::vector<int>>  BeamBkgTag;
    /** Raw energy */
    std::vector<std::vector<double>>  TCRawEnergy;
    /** Raw timing */
    std::vector<std::vector<double>>  TCRawTiming;


    /** Object of TC Mapping */
    TrgEclMapping* _TCMap;
    /** Object of DataBase */
    TrgEclDataBase* _DataBase;

    /** Coeffisients of signal PDF0  */
    std::vector<std::vector<double>> CoeffSigPDF0;
    /** Coeffisients of signal PDF1 */
    std::vector<std::vector<double>> CoeffSigPDF1;
    /** Coeffisients of noise 1 */
    std::vector<std::vector<double>> CoeffNoise31;
    /** Coeffisient of noise 2 */
    std::vector<std::vector<double>> CoeffNoise32;
    /** Coeffisient of noise 3   */
    std::vector<std::vector<double>> CoeffNoise33;
    /** TC Latency */
    std::vector<double>  TCLatency;

    /** Add beambkg */
    int _BeamBkgTag;
    /** Fill Analysis table */
    int _AnaTag;
    /** Threshold (MeV) */
    std::vector<int> Threshold;
    /** Fill Analysis table */
    int EventId;

  };
} // end namespace Belle2

#endif
