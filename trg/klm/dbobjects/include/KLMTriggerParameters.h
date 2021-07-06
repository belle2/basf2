/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/** Belle2 headers. */
#include <framework/logging/Logger.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * DBObject containing parameters used in KLMTrigger module.
   */
  class KLMTriggerParameters : public TObject {

  public:

    /**
     * Default constructor
     */
    KLMTriggerParameters()
    {
    }

    /**
     * Constructor
     * @param[in] nLayers      Number of layers used in the trigger logic.
     * @param[in] whichLayers  Pattern of layers used in the trigger logic.
     */
    KLMTriggerParameters(unsigned int nLayers, const std::string& whichLayers) :
      m_nLayers{nLayers},
      m_whichLayers{whichLayers}
    {
    }

    /**
     * Destructor
     */
    ~KLMTriggerParameters()
    {
    }

    /**
     * Set the number of layers used in the trigger logic.
     * @param[in] nLayers Max number of CDC hits for an event.
     */
    void setNLayers(unsigned int nLayers)
    {
      // Reject values that exceed the number of layers in backward EKLM,
      // since it is the part with the smallest number of layers.
      unsigned int threshold{static_cast<unsigned int>(EKLMElementNumbers::Instance().getMaximalDetectorLayerNumber(EKLMElementNumbers::c_BackwardSection))};
      if (nLayers > threshold)
        B2FATAL("The value passed to 'setNLayers' exceed the maximum allowed number of layers."
                << LogVar("nLayers", nLayers)
                << LogVar("layers threshold", threshold));
      m_nLayers = nLayers;
    }

    /**
     * Set the pattern of layers used in the trigger logic.
     * @param[in] whichLayers Pattern of layers used in the trigger logic.
     */
    void setWhichLayers(const std::string& whichLayers)
    {
      if (whichLayers.empty())
        B2FATAL("The string passed to 'setWhichLayers' is empty."
                << LogVar("string", whichLayers));
      m_whichLayers = whichLayers;
    }

    /**
     * Get the number of layers used in the trigger logic.
     */
    unsigned int getNLayers() const
    {
      return m_nLayers;
    }

    /**
     * Get the pattern of layers used in the trigger logic.
     */
    std::string getWhichLayers() const
    {
      return m_whichLayers;
    }

  private:

    /**
     * Number of layers used in the trigger logic.
     */
    unsigned int m_nLayers{0};

    /**
     * Pattern of layers used in the trigger logic.
     */
    std::string m_whichLayers;

    /**
     * Class version.
     */
    ClassDef(KLMTriggerParameters, 1);

  };

}
