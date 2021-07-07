/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDDatabaseTestModule_H
#define SVDDatabaseTestModule_H


#include <framework/core/Module.h>

#include <svd/calibration/SVDCoGTimeCalibrations.h>

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
      virtual void initialize() override;

      /** Retrieve calibrations from database */
      virtual void event() override;

    private:
      //      SVDNoiseCalibrations m_obj_noise;
      //      SVDPulseShapeCalibrations m_obj_pulseShape;
      SVDCoGTimeCalibrations m_obj_cog; /**< CoG calibrations payload*/
      //      DBObjPtr<SVDLocalRunBadStrips> m_obj_badStrip;



    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDDatabaseTestModule_H
