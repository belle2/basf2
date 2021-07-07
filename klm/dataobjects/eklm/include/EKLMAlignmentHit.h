/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * This dataobject is used only for EKLM alignment. The alignable hits must
   * correspond to only one alignable detector element (EKLM sector in this
   * case). However, the hit EKLMHit2d corresponds to two sectors. Thus, it is
   * not directly usable for alignment. Instead, a new StoreArray of
   * EKLMAlignmentHit is created with 2 entries per each EKLMHit2d. The object
   * EKLMAlignmentHit itself does not contain any data except the digit
   * identifier but has a relation to its EKLMHit2d.
   */
  class EKLMAlignmentHit : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    EKLMAlignmentHit();

    /**
     * Constructor.
     * @param[in] plane Plane number.
     */
    explicit EKLMAlignmentHit(int plane);

    /**
     * Destructor.
     */
    ~EKLMAlignmentHit();

    /**
     * Get digit identifier.
     */
    int getDigitIdentifier() const
    {
      return m_DigitIdentifier;
    }

  private:

    /**
     * Digit identifier (index of digit related to the related EKLMHit2d,
     * 0 or 1).
     */
    int m_DigitIdentifier;

    /** Class version. */
    ClassDef(Belle2::EKLMAlignmentHit, 1);

  };

}
