/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENTHIT_H
#define EKLMALIGNMENTHIT_H

/* Belle2 headers. */
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * This dataobject is used only for EKLM alignment. The alignable hits must
   * correspond to only one alignable detector element (EKLM sector in this
   * case). However, the hit EKLMHit2d corresponds to two sectors. Thus, it is
   * not directly usable for alignment. Instead, a new StoreArray of
   * EKLMAlignmentHit is created with 2 entries per each EKLMHit2d. The object
   * EKLMAlignmentHit itself does not contain any data but has a relation to
   * its EKLMHit2d.
   */
  class EKLMAlignmentHit : public RelationsObject {

  public:

    /**
     * Constructor.
     */
    EKLMAlignmentHit();

    /**
     * Destructor.
     */
    ~EKLMAlignmentHit();

  private:

    /** Needed to make object storable. */
    ClassDef(Belle2::EKLMAlignmentHit, 1);

  };

}

#endif

