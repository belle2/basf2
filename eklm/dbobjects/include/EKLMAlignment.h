/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENT_H
#define EKLMALIGNMENT_H

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TObject.h>

/* Belle2 Headers. */
#include <eklm/dbobjects/EKLMAlignmentData.h>

namespace Belle2 {

  /**
   * Class to store EKLM alignment data in the database.
   */
  class EKLMAlignment : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMAlignment();

    /**
     * Destructor.
     */
    ~EKLMAlignment();

    /**
     * Set alignment data.
     * @param[in] segment Segment number.
     * @param[in] dat     Alignment data.
     */
    void setAlignmentData(uint16_t segment, EKLMAlignmentData* dat);

    /**
     * Get alignment data.
     * @param[in] segment Segment number.
     */
    EKLMAlignmentData* getAlignmentData(uint16_t segment);

    /**
     * Clean alignment data.
     */
    void cleanAlignmentData();

  private:

    /** Alignment data. */
    std::map<uint16_t, EKLMAlignmentData> m_Data;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMAlignment, 1);

  };

}

#endif

