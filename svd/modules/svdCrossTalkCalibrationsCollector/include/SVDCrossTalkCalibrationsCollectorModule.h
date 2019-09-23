/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/CalibrationCollectorModule.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/dataobjects/SVDRecoDigit.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/calibration/SVDOccupancyCalibrations.h>
#include <svd/modules/svdCrossTalkFinder/SVDCrossTalkFinderHelperFunctions.h>

#include <string>
#include <TH2.h>
#include <TFile.h>
#include <map>


namespace Belle2 {

  class SVDCrossTalkCalibrationsCollectorModule : public CalibrationCollectorModule {


  public:

    SVDCrossTalkCalibrationsCollectorModule();

    /** Init the module.*/
    void prepare() override final;

    /** */
    void startRun() override final;

    /** */
    void collect() override final;


    /** */
    void closeRun() override final;


    /** */
    void finish() override;

    TTree* m_histogramTree;
    TH1F* m_hist;
    int m_layer = 4;
    int m_ladder = 16;
    int m_sensor = 5;
    int m_side = 2;

  private:



    void calculateAverage(const VxdID& sensorID, double& mean, int side); /**Function to calculate sensor average occupancy */

    /** SVDRecoDigit collection name. */
    std::string m_svdRecoDigitsName;

    /** The storeArray for svdRecoDigits */
    StoreArray<SVDRecoDigit> m_svdRecoDigits;

    int m_uSideOccupancyFactor; /**Parameter to define high occupancy strips (some multiple above sensor average occupancy) */

    int m_vSideOccupancyFactor; /**Parameter to define high occupancy strips (some multiple above sensor average occupancy) */

    int m_nAPVFactor; /**Parameter to set number of sensors with possible cross-talk clusters required for event flagging.*/

    std::string m_treeName; /** Filename of root file containing cross-talk strip calibration payload */

//    TFile* m_histogramFile; /**Pointer to root TFile containing histograms for calibration payload */

    std::map<std::string, TH1F* > m_sensorHistograms; /**< map to store cross-talk strip histograms */
    SVDOccupancyCalibrations m_OccupancyCal; /**<SVDOccupancy calibrations db object */


  };
}// end namespace Belle2



