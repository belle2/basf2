/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDCrossTalkFinderModule_H
#define SVDCrossTalkFinderModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/calibration/SVDOccupancyCalibrations.h>

#include <TFile.h>
#include <TH1F.h>

#include <string>
#include <map>

namespace Belle2 {


  /** Flags potential cross talk strips on Origami sensors */
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

    /** Function to calculate sensor average occupancy */
    void calculateAverage(const VxdID& sensorID, double& mean, int side);

    /** SVDRecoDigit collection name. */
    std::string m_svdRecoDigitsName;

    /** The storeArray for svdRecoDigits */
    StoreArray<SVDRecoDigit> m_svdRecoDigits;

    /** SVDEventInfo collection name. */
    std::string m_svdEventInfoName;

    /** The storeObject for svdEventInfo */
    StoreObjPtr<SVDEventInfo> m_svdEventInfo;

    int m_uSideOccupancyFactor; /**< Parameter to define high occupancy strips (some multiple above sensor average occupancy) */

    int m_vSideOccupancyFactor; /**< Parameter to define high occupancy strips (some multiple above sensor average occupancy) */

    int m_nAPVFactor; /**< Parameter to set number of sensors with possible cross-talk clusters required for event flagging.*/

    bool m_createCalibrationPayload; /**< If true module will produce and write-out payload for SVDCrossTalkStripsCalibrations. */

    std::string m_outputFilename; /**< Filename of root file containing cross-talk strip calibration payload */

    TFile* m_histogramFile = nullptr; /**< Pointer to root TFile containing histograms for calibration payload */
    std::map<std::string, TH1F* > m_sensorHistograms; /**< map to store cross-talk strip histograms */

    //calibration objects
    SVDOccupancyCalibrations m_OccupancyCal; /**< SVDOccupancy calibrations db object */


  };

} //end namespace Belle2

#endif
