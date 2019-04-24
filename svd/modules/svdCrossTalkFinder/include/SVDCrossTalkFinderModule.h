/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDCrossTalkFinderModule_H
#define SVDCrossTalkFinderModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/dataobjects/SVDRecoDigit.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <string>
#include <TH2.h>
#include <TFile.h>
#include <iostream>
#include <framework/utilities/FileSystem.h>

namespace Belle2 {


  /* Flags potential cross talk strips on Origami sensors */

  class SVDCrossTalkFinderModule : public Module {

  public:

    /* Constructor */
    SVDCrossTalkFinderModule();

    /** Init the module.*/
    virtual void initialize() override;
    /** Event. */
    virtual void event() override;
    /** Final output.*/
    virtual void terminate() override;


  protected:

    void calculateAverage(TH1F* occupancyHist, double& mean); /**Function to calculate sensor average occupancy */

    // Data members

    /** SVDRecoDigit collection name. */
    std::string m_svdRecoDigitsName;

    /** The storeArray for svdRecoDigits */
    StoreArray<SVDRecoDigit> m_svdRecoDigits;

    int m_uSideOccupancyFactor; /**Parameter to define high occupancy strips (some multiple above sensor average occupancy) */

    int m_vSideOccupancyFactor; /**Parameter to define high occupancy strips (some multiple above sensor average occupancy) */

    int m_nAPVFactor; /**Parameter to set number of sensors with possible cross-talk clusters required for event flagging.*/

    std::string m_inputFilePath; /** Filepath of root file containing sensor occupancy sample */

    TFile* m_calibrationFile; /**Pointer to root TFile containing sensor occupancy sample */


  };

} //end namespace Belle2

#endif
