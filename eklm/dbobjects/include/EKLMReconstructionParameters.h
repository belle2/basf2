/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRECONSTRUCTIONPARAMETERS_H
#define EKLMRECONSTRUCTIONPARAMETERS_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to store EKLM simulation in the database.
   */
  class EKLMReconstructionParameters : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMReconstructionParameters();

    /**
     * Destructor.
     */
    ~EKLMReconstructionParameters();

    /**
     * Get time resolution (of reconstructed time, not ADC).
     */
    float getTimeResolution() const;

    /**
     * Set time resolution (of reconstructed time, not ADC).
     */
    void setTimeResolution(float resolution);

  private:

    /** Time resolution. */
    float m_TimeResolution;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMReconstructionParameters, 1);

  };

}

#endif

