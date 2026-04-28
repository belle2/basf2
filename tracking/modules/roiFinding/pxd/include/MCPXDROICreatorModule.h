/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ROIid;
  class MCParticle;
  class PXDDigit;

  /**
   * The MCPXDROICreatorModule module
   * Create artificial ROI just for PXDDigits from either all charged primary MCParticles or slow pions from D* decays on MC information.
   */
  class MCPXDROICreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MCPXDROICreatorModule();

    /** Empty destructor */
    virtual ~MCPXDROICreatorModule() {};

    /** init the module */
    virtual void initialize() override;

    /** processes the event */
    virtual void event() override;

  private:
    std::string m_pxdDigitsName = "";   /**< PXDDigits StoreArray name*/
    std::string m_MCParticlesName = ""; /**< MCParticles StoreArray name*/
    std::string m_ROIsName = "";        /**< ROI StoreArray name*/

    StoreArray<PXDDigit> m_PXDDigits;     /**< StoreArray for the PXDDigits */
    StoreArray<MCParticle> m_MCParticles; /**< StoreArray for the MCParticles */
    StoreArray<ROIid> m_ROIs;             /**< StoreArray for the ROIs */

    short m_ROISize = 40; /**< ROI size */
    bool m_createROIForAll = false; /**< Create ROI for all chargedstable MCParticles */
    bool m_createROIForSlowPionsOnly = true; /**< Create ROI only for slow pions */
  };
}
