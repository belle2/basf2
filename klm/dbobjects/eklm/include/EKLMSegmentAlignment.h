/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>
#include <klm/dbobjects/KLMAlignmentData.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <cstdint>
#include <map>

namespace Belle2 {

  /**
   * Class to store EKLM alignment data in the database.
   */
  class EKLMSegmentAlignment : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMSegmentAlignment();

    /**
     * Destructor.
     */
    ~EKLMSegmentAlignment();

    /**
     * Set segment alignment data.
     * @param[in] segment Segment number.
     * @param[in] dat     Alignment data.
     */
    void setSegmentAlignment(EKLMSegmentNumber segment, KLMAlignmentData* dat);

    /**
     * Get segment alignment data.
     * @param[in] segment Segment number.
     */
    const KLMAlignmentData* getSegmentAlignment(EKLMSegmentNumber segment) const;

    /* Interface to global Millepede calibration. */

    /**
     * Get global unique identifier.
     * @return Global unique identifier.
     */
    static unsigned short getGlobalUniqueID()
    {
      return 41;
    }

    /**
     * Get global parameter.
     * @param[in] element Element number (EKLM segment number).
     * @param[in] param   Parameter number.
     * @return Global parameter value.
     */
    double getGlobalParam(unsigned short element, unsigned short param) const;

    /**
     * Set global parameter.
     * @param[in] value   Parameter value.
     * @param[in] element Element number (EKLM segment number).
     * @param[in] param   Parameter number.
     */
    void setGlobalParam(double value, unsigned short element,
                        unsigned short param);

    /**
     * Get a list of stored global parameters.
     */
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams();

  private:

    /** Segment alignment. */
    std::map<EKLMSegmentNumber, KLMAlignmentData> m_SegmentAlignment;

    /** Class version. */
    ClassDef(Belle2::EKLMSegmentAlignment, 1);

  };

}
