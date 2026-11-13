/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLFAMFIT_H
#define TRGECLFAMFIT_H

#include "trg/ecl/TrgEclMapping.h"
#include <trg/ecl/TrgEclDataBase.h>

#include <array>

namespace Belle2 {

  /*! FAM module   */
  class TrgEclFAMFit {

  private:
    /// Number of ECL TriggerCells (TC)
    static constexpr int c_NTC = 576;

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
    void setThreshold(const std::array<int, c_NTC>& tcethreshold)
    {
      m_TCEThreshold = tcethreshold;
    };
    /** set Beam Background Tag */
    void setBeamBkgTag();
    /** Clear vectors */
    void clear();

  private:
    /** fit energy */
    std::array<std::vector<double>, c_NTC>  m_TCFitEnergy;
    /** fit timing */
    std::array<std::vector<double>, c_NTC>  m_TCFitTiming;
    /** fit timing */
    std::array<std::vector<int>, c_NTC>     m_BeamBkgInfo;
    /** Raw energy */
    std::array<std::vector<double>, c_NTC>  m_TCRawEnergy;
    /** Raw timing */
    std::array<std::vector<double>, c_NTC>  m_TCRawTiming;


    /** Object of TC Mapping */
    TrgEclMapping* m_TCMap;
    /** Object of DataBase */
    TrgEclDataBase* m_DataBase;

    /** Coeffisients of signal PDF0  */
    std::array<std::array<double, 12>, 20> m_CoeffSigPDF0;
    /** Coeffisients of signal PDF1 */
    std::array<std::array<double, 12>, 20> m_CoeffSigPDF1;
    /** Coeffisients of noise 1 */
    std::array<std::array<double, 12>, 20> m_CoeffNoise31;
    /** Coeffisient of noise 2 */
    std::array<std::array<double, 12>, 20> m_CoeffNoise32;
    /** Coeffisient of noise 3   */
    std::array<std::array<double, 12>, 20> m_CoeffNoise33;

    /** Add beambkg */
    int m_BeamBkgTag = 0;
    /** Fill Analysis table */
    int m_AnaTag = 0;
    /** Threshold (MeV) */
    std::array<int, c_NTC> m_TCEThreshold;
    /** Fill Analysis table */
    int m_EventId = 0;

  };
} // end namespace Belle2

#endif
