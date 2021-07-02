/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>
#include <klm/dbobjects/KLMAlignmentData.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <cstdint>
#include <map>

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
    void setModuleAlignment(KLMModuleNumber module, KLMAlignmentData* dat);

    /**
     * Get module alignment data.
     * @param[in] module Module number.
     */
    const KLMAlignmentData* getModuleAlignment(KLMModuleNumber module) const;

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
     * @param[in] element Element number (KLM module number).
     * @param[in] param   Parameter number.
     * @return Global parameter value.
     */
    double getGlobalParam(unsigned short element, unsigned short param) const;

    /**
     * Set global parameter.
     * @param[in] value   Parameter value.
     * @param[in] element Element number (KLM module number).
     * @param[in] param   Parameter number.
     */
    void setGlobalParam(double value, unsigned short element,
                        unsigned short param);
    /**
     * Get a list of stored global parameters.
     */
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams();

  private:

    /** Module alignment. */
    std::map<KLMModuleNumber, KLMAlignmentData> m_ModuleAlignment;

    /** Class version. */
    ClassDef(Belle2::BKLMAlignment, 3);

  };

}
