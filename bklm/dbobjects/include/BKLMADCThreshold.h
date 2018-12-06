/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMADCTHRESHOLD_DB_H
#define BKLMADCTHRESHOLD_DB_H

#include <TObject.h>
#include <string>
#include <cmath>
#include <TROOT.h>
#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for BKLM scintillator ADC related parameters.
   * including the pedestal, ADC threshold , gain at least.
   * TO do: set the fine tuned parameters for each scintillator channel
   */

  class BKLMADCThreshold: public TObject {
  public:

    //! Default constructor
    BKLMADCThreshold():  m_ADCOffset(0), m_threshold(0), m_MPPCGain(0) {};

    //!Constructor
    BKLMADCThreshold(int ADCOffset, double threshold, double gain)
    {
      m_ADCOffset = ADCOffset;
      m_threshold = threshold;
      m_MPPCGain = gain;
    };

    //! Destructor
    ~BKLMADCThreshold() {};

    //! Get the ADC offset
    int getADCOffset() const {return m_ADCOffset;}

    //! Get the ADC threshold
    double getADCThreshold() const { return m_threshold; }

    //! Get the MPPC gain
    double getMPPCGain() const { return m_MPPCGain; }

    //! Set the ADC offset
    void setADCOffset(int ADCOffset) { m_ADCOffset = ADCOffset;}

    //! Set the ADC threshold
    void setADCThreshold(double threshold) { m_threshold = threshold; }

    //! Set the MPPC gain
    void setMPPCGain(double gain) { m_MPPCGain = gain; }


  private:

    //! ADC offset/pedestal
    int m_ADCOffset;

    //! ADC threshold
    double m_threshold;

    //! MPPC gain
    double m_MPPCGain;

    //! ClassDef, must be the last term before the closing {}
    ClassDef(BKLMADCThreshold, 1);

  };

} // end namespace Belle2

#endif
