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
#include <klm/dataobjects/KLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHitBase.h>
#include <klm/eklm/dataobjects/EKLMHitMCTime.h>
#include <klm/eklm/dataobjects/EKLMSimHit.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>

/* Belle 2 headers. */
#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {

  /**
   * Main reconstruction hit class. Contains information about the
   * hitted strips.
   */
  class EKLMDigit : public KLMDigit,
    public EKLMHitGlobalCoord, public EKLMHitMCTime {

  public:

    /**
     * Constructor.
     */
    EKLMDigit();

    /**
     * Constructor from the EKLMSim2Hit.
     * @param[in] Hit EKLMSim2Hit.
     */
    explicit EKLMDigit(const EKLMSimHit* Hit);

    /**
     * Destructor.
     */
    ~EKLMDigit() {};

    /**
     * Get unique channel identifier.
     */
    unsigned int getUniqueChannelID() const override;

    /**
     * The pile-up method.
     */
    EAppendStatus addBGDigit(const DigitBase* bg) override;

  private:

    /** Class version. */
    ClassDefOverride(Belle2::EKLMDigit, 12);

  };

}
