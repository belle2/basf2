/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/** Basf2 headers. */
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
    KLMTriggerParameters(int32_t nLayers, const std::string& whichLayers) :
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
    void setNLayers(int32_t nLayers)
    {
      // Reject values that exceed the number of layers in backward EKLM,
      // since it is the part with the smallest number of layers.
      int32_t threshold{static_cast<int32_t>(EKLMElementNumbers::Instance().getMaximalDetectorLayerNumber(EKLMElementNumbers::c_BackwardSection))};
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
    int32_t getNLayers() const
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

    int32_t getSubdetector(int32_t i) const
    {
      return m_Subdetector[i];
    }

    int32_t getSection(int32_t i) const
    {
      return m_section[i];
    }
    int32_t getSector(int32_t i) const
    {
      return m_sector[i];
    }
    int32_t getLayer(int32_t i) const
    {
      return m_layer[i];
    }

    int32_t getPlane(int32_t i) const
    {
      return m_plane[i];
    }

    double getSlopeX(int32_t i) const
    {
      return m_slopeX[i];
    }
    double getOffsetX(int32_t i) const
    {
      return m_offsetX[i];
    }

    double getSlopeY(int32_t i) const
    {
      return m_slopeY[i];
    }
    double getOffsetY(int32_t i) const
    {
      return m_offsetY[i];
    }


    size_t getGeometryDataSize() const
    {
      return m_Subdetector.size();
    }

    void pushGeometryData(int32_t Subdetector, int32_t section, int32_t sector, int32_t layer, int32_t plane, double  slopeX,
                          double offsetX,
                          double  slopeY, double offsetY)
    {

      m_Subdetector.push_back(Subdetector);
      m_section.push_back(section);
      m_sector.push_back(sector);
      m_layer.push_back(layer);
      m_plane.push_back(plane);
      m_slopeX.push_back(slopeX);
      m_offsetX.push_back(offsetX);
      m_slopeY.push_back(slopeY);
      m_offsetY.push_back(offsetY);
    }


  private:

    /**
     * Number of layers used in the trigger logic.
     */
    int32_t m_nLayers{0};

    std::vector<int32_t> m_Subdetector;
    std::vector<int32_t> m_section;
    std::vector<int32_t> m_sector;
    std::vector<int32_t> m_layer;
    std::vector<int32_t> m_plane;
    std::vector<double> m_slopeX;
    std::vector<double> m_offsetX;
    std::vector<double> m_slopeY;
    std::vector<double> m_offsetY;



    /**
     * Pattern of layers used in the trigger logic.
     */
    std::string m_whichLayers;

    /**
     * Class version.
     */
    ClassDef(KLMTriggerParameters, 3);

  };

}
