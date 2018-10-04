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

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMElementID.h>
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
     * Set sector alignment data.
     * @param[in] sector Sector number.
     * @param[in] dat    Alignment data.
     */
    void setSectorAlignment(uint16_t sector, EKLMAlignmentData* dat);

    /**
     * Get sector alignment data.
     * @param[in] sector Sector number.
     */
    const EKLMAlignmentData* getSectorAlignment(uint16_t sector) const;

    /**
     * Set segment alignment data.
     * @param[in] segment Segment number.
     * @param[in] dat     Alignment data.
     */
    void setSegmentAlignment(uint16_t segment, EKLMAlignmentData* dat);

    /**
     * Get segment alignment data.
     * @param[in] segment Segment number.
     */
    const EKLMAlignmentData* getSegmentAlignment(uint16_t segment) const;

    /* Interface to global Millepede calibration. */

    /**
     * Get global unique identifier.
     * @return Global unique identifier.
     */
    static unsigned short getGlobalUniqueID() { return 40; }

    /**
     * Get global parameter.
     * @return Global parameter value.
     */
    double getGlobalParam(unsigned short element, unsigned short param) const;

    /**
     * Set global parameter.
     */
    void setGlobalParam(double value, unsigned short element,
                        unsigned short param);
    /**
     * Get a list of stored global parameters.
     */
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams();

  private:

    /** Sector alignment. */
    std::map<uint16_t, EKLMAlignmentData> m_SectorAlignment;

    /** Segment alignment. */
    std::map<uint16_t, EKLMAlignmentData> m_SegmentAlignment;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMAlignment, 2);

  };

}

#endif

