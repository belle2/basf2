/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDDatabaseTestModule_H
#define SVDDatabaseTestModule_H


#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>

#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>
#include <svd/dbobjects/SVDLocalRunBadStrips.h>

#include <vxd/dataobjects/VxdID.h>


namespace Belle2 {

  namespace SVD {

    /** Return the calibration results for the noises and the
     * constants measured during the svd local runs
     * (charge, ADC pulse, peaking time, pulse width). It also
     * provides the status of bad strips, as flagged
     * during the local runs.
     *
     */
    class SVDDatabaseTestModule : public Module {

    public:

      /** Constructor */
      SVDDatabaseTestModule();

      /** Initialize the module */
      virtual void initialize();

      /** Retrieve calibrations from database */
      virtual void event();

    private:
      //      SVDNoiseCalibrations m_obj_noise;
      //      SVDPulseShapeCalibrations m_obj_pulseShape;
      SVDCoGTimeCalibrations m_obj_cog;
      //      DBObjPtr<SVDLocalRunBadStrips> m_obj_badStrip;



    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDDatabaseTestModule_H
