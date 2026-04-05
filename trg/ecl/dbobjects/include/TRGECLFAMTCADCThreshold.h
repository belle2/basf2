/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLFAMTCADCTHRESHOLD_H
#define TRGECLFAMTCADCTHRESHOLD_H

#include <TObject.h>

namespace Belle2 {

  //! DB object to store TC ADC Threshold
  class TRGECLFAMTCADCThreshold : public TObject {
  public:

    //! Constructor
    TRGECLFAMTCADCThreshold() :
      m_TCId(0),
      m_TCADCThreshold(0)
    {}
    //! Constructor
    TRGECLFAMTCADCThreshold(int TCId,
                            int TCADCThreshold):
      m_TCId(TCId),
      m_TCADCThreshold(TCADCThreshold)
    {}

    //! Set TCId
    void setTCId(int TCId)
    {
      m_TCId = TCId;
    }
    //! Set TC ADC Threshold (ADC)
    void setTCADCThreshold(int TCADCThreshold)
    {
      m_TCADCThreshold = TCADCThreshold;
    }
    //! Get TCId
    int getTCId() const
    { return m_TCId ; }
    //! Get TC ADC Threshold (ADC)
    int getTCADCThreshold() const
    { return m_TCADCThreshold ; }

    //! TCId
    int m_TCId;
    //! TC ADC Threshold (ADC)
    int m_TCADCThreshold;

    //! the class title
    ClassDef(TRGECLFAMTCADCThreshold, 1); /*< the class title */
  };
} //! end namespace Belle2

#endif

