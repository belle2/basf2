/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
    /** set Beam Background Tag flag */
    void setBeamBkgTagFlag(int beambkgtagflag) { m_BeamBkgTag = beambkgtagflag; }
    /** set flag for saving analysis table*/
    void setAnaTagFlag(int anatagflag) { m_AnaTag = anatagflag; }
    /** set Threshold */
    void setThreshold(const std::vector<int>& tcethreshold)
    {
      m_TCEThreshold = tcethreshold;
    };
    /** set Beam Background Tag */
    void setBeamBkgTag();

    //! Get TC Energy
    std::vector<std::vector<double>>  getFitEnergy() {return m_TCFitEnergy;}
    //! Get TC Timing
    std::vector<std::vector<double>>  getFitTiming() {return m_TCFitTiming;}
    //! Get Background Tag of TC Hit
    std::vector<std::vector<int>>     getBeamBkgTag() {return m_BeamBkgInfo;}

  private:

    /** fit energy */
    std::vector<std::vector<double>>  m_TCFitEnergy;
    /** fit timing */
    std::vector<std::vector<double>>  m_TCFitTiming;
    /** fit timing */
    std::vector<std::vector<int>>     m_BeamBkgInfo;
    /** Raw energy */
    std::vector<std::vector<double>>  m_TCRawEnergy;
    /** Raw timing */
    std::vector<std::vector<double>>  m_TCRawTiming;


    /** Object of TC Mapping */
    TrgEclMapping* m_TCMap;
    /** Object of DataBase */
    TrgEclDataBase* m_DataBase;

    /** Coeffisients of signal PDF0  */
    std::vector<std::vector<double>> m_CoeffSigPDF0;
    /** Coeffisients of signal PDF1 */
    std::vector<std::vector<double>> m_CoeffSigPDF1;
    /** Coeffisients of noise 1 */
    std::vector<std::vector<double>> m_CoeffNoise31;
    /** Coeffisient of noise 2 */
    std::vector<std::vector<double>> m_CoeffNoise32;
    /** Coeffisient of noise 3   */
    std::vector<std::vector<double>> m_CoeffNoise33;
    /** TC Latency */
    std::vector<double>  m_TCLatency;

    /** Add beambkg */
    int m_BeamBkgTag;
    /** Fill Analysis table */
    int m_AnaTag;
    /** Threshold (MeV) */
    std::vector<int> m_TCEThreshold;
    /** Fill Analysis table */
    int m_EventId;

  };
} // end namespace Belle2

#endif
