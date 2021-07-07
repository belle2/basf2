/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {

  /** The ROI generator Module
   *
   * This module is used to generate a certain number of ROIs
   * mainly for debugging purpose
   */

  class ROIGeneratorModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROIGeneratorModule();

  private:

    void initialize() override final;

    void event() override final;


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

    bool m_random; /**< move single roi pseudo randomly */

  };
}

