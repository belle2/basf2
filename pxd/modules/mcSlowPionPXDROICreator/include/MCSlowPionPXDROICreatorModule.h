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
   * The MCSlowPionPXDROICreatorModule module
   * Create artificial ROI just for PXDDigits from slow pions from D* decays.
   */
  class MCSlowPionPXDROICreatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MCSlowPionPXDROICreatorModule();

    /** Empty destructor */
    virtual ~MCSlowPionPXDROICreatorModule() {};

    /** init the module */
    virtual void initialize() override;

    /** processes the event */
    virtual void event() override;

    /** end the run */
    virtual void endRun() override;

  private:
    std::string m_pxdDigitsName = "";   /**< PXDDigits StoreArray name*/
    std::string m_MCParticlesName = ""; /**< MCParticles StoreArray name*/
    std::string m_ROIsName = "";        /**< ROI StoreArray name*/

    StoreArray<PXDDigit> m_PXDDigits;     /**< StoreArray for the PXDDigits */
    StoreArray<MCParticle> m_MCParticles; /**< StoreArray for the MCParticles */
    StoreArray<ROIid> m_ROIs;             /**< StoreArray for the ROIs */

    short m_ROISize = 10; /**< ROI size */

    uint m_slowPiCounter = 0; /**< Count the slow pions in MC for debugging*/
    uint m_ROICounter = 0;    /**< Count the ROI that are created for debugging */
  };
}
