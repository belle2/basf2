/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMSimHit.h>
#include <klm/dataobjects/KLMDigit.h>

namespace Belle2 {

  /**
   * Main reconstruction hit class. Contains information about the
   * hitted strips.
   */
  class EKLMDigit : public KLMDigit {

  public:

    /**
     * Constructor.
     */
    EKLMDigit() : KLMDigit()
    {
    }

    /**
     * Constructor from the EKLMSim2Hit.
     * @param[in] hit EKLMSim2Hit.
     */
    explicit EKLMDigit(const EKLMSimHit* hit) : KLMDigit(hit)
    {
    }

    /**
     * Destructor.
     */
    ~EKLMDigit() {};

  private:

    /** Class version. */
    ClassDefOverride(Belle2::EKLMDigit, 12);

  };

}
