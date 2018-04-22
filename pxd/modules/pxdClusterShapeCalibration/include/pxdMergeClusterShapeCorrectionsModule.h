/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef pxdMergeClusterShapeCorrectionsModule_H
#define pxdMergeClusterShapeCorrectionsModule_H

#include <framework/core/Module.h>
#include <pxd/calibration/PXDClusterShapeCalibrationAlgorithm.h>
#include <TFile.h>

namespace genfit {
  class Track;
}

namespace Belle2 {

  namespace PXD {

    /** The pxdMergeClusterShapeCorrections module.
      *
      * Merge PXD Cluster Shape Calibrations from 4 + 1 correction files to one.
      * 4 files are special for full range of angles from simulations for 4 types of pixels (different pich size in v)
      * 1 file is from real data or simulation on Belle II geometry with only real range of angles. TODO less priority now, no data.
      *
      * @see PXDClusterShape
      * @see PXDClusterShapeCalibration
      * @see PXDClusterShapeCalibrationAlgorithm
      * @see PXDDQMClusterShape
      *
      */
    class pxdMergeClusterShapeCorrectionsModule : public Module {

    public:
      /** Constructor defining the parameters */
      pxdMergeClusterShapeCorrectionsModule();
      /** Initialize the module */
      virtual void initialize();
      /** do the clustering */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    private:

      /** Name of file contain output merged calibration, default = PXD-ClasterShapeCorrections */
      std::string m_CalFileOutputName = "PXD-ClasterShapeCorrections.root";
      /** Name of file contain basic calibration, default = "pxdClShCal_RealData0_Pixel-1 */
      std::string m_CalFileBasicName = "pxdClShCal_RealData0_Pixel-1.root";
      /** Name of file contain calibration for pixel kind 0 (PitchV = 55um), default = pxdClShCal_RealData0_Pixel0 */
      std::string m_CalFilePK0Name = "pxdClShCal_RealData0_Pixel0.root";
      /** Name of file contain calibration for pixel kind 1 (PitchV = 60um), default = pxdClShCal_RealData0_Pixel1 */
      std::string m_CalFilePK1Name = "pxdClShCal_RealData0_Pixel1.root";
      /** Name of file contain calibration for pixel kind 2 (PitchV = 70um), default = pxdClShCal_RealData0_Pixel2 */
      std::string m_CalFilePK2Name = "pxdClShCal_RealData0_Pixel2.root";
      /** Name of file contain calibration for pixel kind 3 (PitchV = 85um), default = pxdClShCal_RealData0_Pixel3 */
      std::string m_CalFilePK3Name = "pxdClShCal_RealData0_Pixel3.root";

      /** Only over this limit is bias correction accepted, default = 1.0 um, in [cm] */
      float m_Difference = 1.0 * Unit::um;
      /** Only under this limit is real bias correction compare to simulation accepted, default = 3.0 um, in [cm] */
      float m_DifferenceClose = 3 * Unit::um;
      /** Only under this limit is real error estimation correction compare to simulation accepted, default = 0.1 um, in [cm] */
      float m_DifferenceErrEst = 0.1;
      /** Create and fill reference histograms in DataBase, default = 0 */
      int m_CreateDB = 0;

    };//end class declaration

  } //end PXD namespace;
} // end namespace Belle2

#endif // pxdMergeClusterShapeCorrectionsModule_H
