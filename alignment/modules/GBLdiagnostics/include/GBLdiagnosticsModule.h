/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GBLDIAGNOSTICSMODULE_H
#define GBLDIAGNOSTICSMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <genfit/Track.h>
#include <genfit/GblFitStatus.h>
#include <genfit/GblFitterInfo.h>
#include <genfit/TrackPoint.h>
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{ GBLdiagnosticsModule @}
   */

  /**
   * Loops over genfit::Tracks fitted by GBLFitter and produces ROOT file with tree(s) with fit result data.
   *
   * You need to have tracks fitted by GBLfit module. This module then scans
   * results of GBL fit in all track and all points. For VXD sensors, you get tree
   * of data separated by sensor, ladder, layer, wire.
   * Pure scattering points are identified by id of preceeding sensor with sensor (or wire) id negative.
   * You also get fitted states, residuals, errors etc.
   *
   */
  class GBLdiagnosticsModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    GBLdiagnosticsModule();

    /** Creates output ROOT file and sets branches */
    virtual void initialize();

    /** Outputs event data to trees */
    virtual void event();

    /** Close the ROOT file */
    virtual void terminate();


  private:


    /** ROOT File to write information to */
    TFile* m_file {0};
    /** ROOT Tree for point level data */
    TTree* m_tree {0};
    /** ROOT Tree for track level data */
    TTree* m_trackTree {0};

    /** struct with all the branches needed */
    struct point_info {
      /** id of the sensor for the current hit */
      int   sensorID;
      /** pdg code for the current track */
      int   pdg;
      /**  layer */
      int   layer;
      /**  ladder */
      int   ladder;
      /**  sensor */
      int   sensor;
      /**  wire */
      int   wire;

      /**  thin scatterer variance */
      double thinScat;

      /** local prediction before kink */
      double predB[5];

      /** local prediction after kink */
      double predF[5];

      /** global prediction before kink */
      double predBglo[6];

      /** global prediction after kink */
      double predFglo[6];

      /** residuals */
      double res[2];

      /** residual errors */
      double resErr[2];

      /** kinks */
      double kink[2];

      /** kink errors */
      double kinkErr[2];

      /** helix params */
      double helix[5];

      int clsSizeU;
      int clsSizeV;
      int clsSize;
      int run;
      /**  */
      long timestamp;
      int id;
      int trackid;
      int   ndf;
      /**  */
      double  chi2;
      /**  */
      double  pval;
      double  mom;
      double  phi;
      double  theta;

    } m_info; /**< object to store all variables */


    /** struct with all the branches needed */
    struct track_info {
      int   pdg;
      /**  */
      int   ndf;
      /**  */
      double  chi2;
      /**  */
      double  pval;
      /**  */
      int   npoints;
      /**  */
      int nmeas;
      /**  */
      int nscat;
      int run;
      /**  */
      long timestamp;
      int id;
      double pred[6];

      /** helix params */
      double helix[5];

    } m_tinfo; /**< object to store all variables */


    int ntracks;

    std::string m_tracksColName;  /**< Name of collection with genfit::Track fitted by GBL */
    std::string m_rootFile;  /**< Name of output ROOT file with trees with fit data */
  };
}

#endif /* GBLDIAGNOSTICSMODULE_H */
