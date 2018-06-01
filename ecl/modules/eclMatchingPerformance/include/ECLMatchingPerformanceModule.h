/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Frank Meier                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ECLMATCHINGPERFORMANCEMODULE_H_
#define ECLMATCHINGPERFORMANCEMODULE_H_

#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/geometry/ECLNeighbours.h>
#include <framework/core/Module.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/modules/standardTrackingPerformance/ParticleProperties.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;
  class ExtHit;
  class ECLCluster;

  template< class T >
  class StoreArray;


  /** This module takes the MCParticle and the genfit::Track collection as input and
   * writes out a root file with some information of the reconstructed tracks.
   * If a generated track is not reconstructed, all output variables are set to
   * the default value (-999). With the output file, you are able to estimate the
   * reconstruction efficiency of tracks
   */
  class ECLMatchingPerformanceModule : public Module {
  public:
    ECLMatchingPerformanceModule();

    /** Register the needed StoreArrays and open the output TFile. */
    void initialize();

    /** Fill the tree with the event data.  */
    void event();

    /** Write the tree into the opened root file. */
    void terminate();

  private:
    std::string m_outputFileName; /**< name of output root file */

    StoreArray<ECLCalDigit> m_eclCalDigits;
    StoreArray<ECLCluster> m_eclClusters;
    StoreArray<ExtHit> m_extHits;
    StoreArray<RecoTrack> m_recoTracks;
    StoreArray<Track> m_tracks;
    StoreArray<TrackFitResult> m_trackFitResults;

    ECL::ECLNeighbours* m_eclNeighbours1x1;
    ECL::ECLNeighbours* m_eclNeighbours3x3;
    ECL::ECLNeighbours* m_eclNeighbours5x5;

    TFile* m_outputFile; /**< output root file */
    TTree* m_dataTree; /**< root tree with all output data. Tree will be written to the output root file */

    /**< properties of a reconstructed track */
    ParticleProperties m_trackProperties;

    /**< Experiment number */
    int m_iExperiment;

    /**< Run number */
    int m_iRun;

    /**< Event number */
    int m_iEvent;

    /**< Track Multiplicity */
    int m_trackMultiplicity;

    /**< pValue of track fit */
    double m_pValue;

    /**< charge */
    int m_charge;

    /**< signed distance of the track to the IP in the r-phi plane */
    double m_d0;

    /**< distance of the track to the IP along the beam axis */
    double m_z0;

    /**< boolean for match between track and ECL cluster */
    int m_matchedToECLCluster;

    /**< hypothesis of matched ECL cluster */
    int m_hypothesisOfMatchedECLCluster;

    /**< minimal distance between track and ECLCluster */
    double m_distance;

    /**< length of track in ECL */
    double m_trackLength;

    /**< energy loss of track in ECL */
    double m_deposited_energy;

    /**< minimal distance between track at center of ECL and ECLCalDigit with at least 10 MeV */
    double m_innerdistance;

    /**< boolean for match between track and ECLCalDigit cell */
    int m_matchedTo1x1Neighbours;

    /**< boolean for match between track and one of 9 ECLCalDigit neighbouring cells */
    int m_matchedTo3x3Neighbours;

    /**< boolean for match between track and one of 25 ECLCalDigit neighbouring cells */
    int m_matchedTo5x5Neighbours;

    /**< boolean for match between track and neighbouring cell with lower phi value */
    int m_matchedToDecreasedPhi;

    /**< boolean for match between track and neighbouring cell with higher phi value */
    int m_matchedToIncreasedPhi;

    /**< boolean for match between track and neighbouring cell with lower phi value */
    int m_matchedToDecreasedTheta;

    /**< boolean for match between track and neighbouring cell with higher phi value */
    int m_matchedToIncreasedTheta;

    /**< boolean for match between track and neighbouring cell with lower phi value */
    int m_matchedToDecreasedPhiDecreasedTheta;

    /**< boolean for match between track and neighbouring cell with higher phi value */
    int m_matchedToIncreasedPhiDecreasedTheta;

    /**< boolean for match between track and neighbouring cell with lower phi value */
    int m_matchedToDecreasedPhiIncreasedTheta;

    /**< boolean for match between track and neighbouring cell with higher phi value */
    int m_matchedToIncreasedPhiIncreasedTheta;

    /**< number of times track enters ECL */
    int m_enter;

    /**< number of times track leaves ECL */
    int m_exit;

    /**< cell id of first crystal that is entered by track */
    int m_enteringcellid;

    /**< theta of first crystal that is entered by track */
    double m_enteringcelltheta;

    /**< number of degrees of freedom of the track (should be #CDC hits - 5 (helix parameters)) */
    int m_ndf;

    /**
     * Sets all variables to the default value, here -999.
     * */
    void setVariablesToDefaultValue();

    /** add branches to data tree */
    void setupTree();

    /** write root tree to output file and close the file */
    void writeData();

    /** add a variable with double format */
    void addVariableToTree(const std::string& varName, double& varReference);

    /** add a variable with int format */
    void addVariableToTree(const std::string& varName, int& varReference);

    /** find a match between crystals in which energy was deposited and the cell or its neighbors that a track entered  */
    void findECLCalDigitMatchInNeighbouringCell(ECL::ECLNeighbours* eclneighbours, int& matchedToNeighbours, const int& cell);

    /** determine whether energy has been deposited in crystal with ID cell and write result to matched */
    void findECLCalDigitMatch(const int& cell, int& matched);
  };


} // end of namespace



#endif /* ECLMATCHINGPERFORMANCEMODULE_H_ */
