/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROI_GENERATOR_H_
#define ROI_GENERATOR_H_

#include <framework/core/Module.h>

namespace Belle2 {

  /** The ROI generator Module
   *
   * This module is used to generate a certain number of ROIs
   *
   */

  class ROIGeneratorModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROIGeneratorModule();

    /**
     *Initializes the Module.
     */
    void initialize() override;

    void event() override;

  protected:

    std::string m_ROIListName; /**< ROi list name*/
    int m_divider; /**< generate one ROI every m_divider event*/
    int m_layer  ; /**< layer */
    int m_ladder ; /**< ladder*/
    int m_sensor ; /**< sensor*/

    int m_minU   ; /**< min U*/
    int m_maxU   ; /**< max U*/

    int m_minV   ; /**< min V*/
    int m_maxV   ; /**< max V*/


    int m_nROIs; /**< number of ROIs per event*/

  };
}
#endif
