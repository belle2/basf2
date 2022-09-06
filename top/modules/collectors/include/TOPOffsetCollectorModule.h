/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/BunchStructure.h>
#include <framework/gearbox/Const.h>
#include <map>

namespace Belle2 {

  /**
   * Collector for eventT0 and fill pattern offset calibrations
   */
  class TOPOffsetCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPOffsetCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here
     */
    virtual void collect() final;

    // collections
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreObjPtr<EventT0> m_eventT0; /**< event T0 */

    // database
    DBObjPtr<BunchStructure> m_bunchStructure;  /**< fill pattern */

    // other
    std::map<Const::EDetector, std::string> m_names; /**< histogram names */
    bool m_firstEvent = true; /**< flag */

  };

} // end namespace Belle2
