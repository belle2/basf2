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

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TObject.h>

/* Belle2 headers. */
#include <klm/dbobjects/KLMAlignmentData.h>

namespace Belle2 {

  /**
   * Class to store BKLM alignment data in the database.
   */
  class BKLMAlignment : public TObject {

  public:

    /**
     * Constructor.
     */
    BKLMAlignment();

    /**
     * Destructor.
     */
    ~BKLMAlignment();

    /**
     * Set module alignment data.
     * @param[in] module Module number.
     * @param[in] dat    Alignment data.
     */
    void setModuleAlignment(uint16_t module, KLMAlignmentData* dat);

    /**
     * Get module alignment data.
     * @param[in] module Module number.
     */
    const KLMAlignmentData* getModuleAlignment(uint16_t module) const;

    /* Interface to global Millepede calibration. */

    /**
     * Get global unique identifier.
     * @return Global unique identifier.
     */
    static unsigned short getGlobalUniqueID()
    {
      return 30;
    }

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

    /** Module alignment. */
    std::map<uint16_t, KLMAlignmentData> m_ModuleAlignment;

    /** Class version. */
    ClassDef(Belle2::BKLMAlignment, 3);

  };

}
