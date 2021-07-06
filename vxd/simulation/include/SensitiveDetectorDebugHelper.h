/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef VXD_SIMULATION_SENSITIVEDETECTORDEBUGHELPER_H
#define VXD_SIMULATION_SENSITIVEDETECTORDEBUGHELPER_H

#include <vxd/simulation/SensorTraversal.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/dataobjects/VXDTrueHit.h>
#include <vxd/dataobjects/VXDSimHit.h>

#include <TFile.h>
#include <TTree.h>

namespace Belle2 {
  namespace VXD {

    /** Small helper class to facilitate debugging of VXD::SensitiveDetector
     * implementation.
     *
     * This file will write a root file containing the Geant4 Step information as
     * well as the information used for TrueHit and SimHit creation to check
     * whether these are consistent. Normally this class should not be in use, a
     * define can be set in VXD::SensitiveDetectorBase to enable its use.
     *
     * This class is designed to make access very easy in python using numpy,
     * e.g. the SimHits can be obtained as numpy array from the tree using
     * >>> tree.GetEntry(i)
     * >>> simhits = np.array(tree.simhitInfo).reshape((-1, 13))  # HIT_SIZE=13
     */
    class SensitiveDetectorDebugHelper {
    public:
      /** Singleton class: get instance */
      static SensitiveDetectorDebugHelper& getInstance();
      /** Start writing a new sensor traversal: Geant4 steps will be added to the ROOT entry */
      void startTraversal(VxdID sensorID, const SensorTraversal& traversal);
      /** Write the information normally used when creating TrueHits to the entry */
      void addTrueHit(const VXDTrueHit* truehit);
      /** Write the information normally used when creating SimHits to the entry */
      void addSimHit(const VXDSimHit* simhit, int startPoint, int endPoint);
      /** Finish the entry by calling Fill() */
      void finishTraversal()
      {
        m_tree->Fill();
      }
    private:
      /** Singleton class: private constructor */
      SensitiveDetectorDebugHelper();
      /** Singleton class: no copy constructor */
      SensitiveDetectorDebugHelper(const SensitiveDetectorDebugHelper&) = delete;
      /** Singleton class: no assignment operator */
      SensitiveDetectorDebugHelper& operator=(const SensitiveDetectorDebugHelper&) = delete;
      /** Destructor: write root file */
      ~SensitiveDetectorDebugHelper()
      {
        m_tree->Write();
        m_file->Close();
      }

      /** ROOT File to write information to */
      TFile* m_file {0};
      /** ROOT Tree to write information to */
      TTree* m_tree {0};

      enum {
        /** assume maximum number of steps per sensor traversal */
        MAX_STEPS = 5000,
        /** assume maximum number of simhits per sensor traversal */
        MAX_HITS = 500,
        /** assume maximum number of energy deposition points per sensor traversal */
        MAX_EDEP = 10000,

        /** number of values per step */
        SIZE_STEP = 9,
        /** number of values per truehit */
        SIZE_TRUE = 20,
        /** number of values per simhit */
        SIZE_HITS = 13,
        /** number of values per energy deposition point */
        SIZE_EDEP = 2
      };

      /** struct with all the branches needed */
      struct info {
        /** id of the sensor for the current track */
        int   sensorID;
        /** pdg code for the current track */
        int   pdg;
        /** whether or not the track was contained in the senitive volume */
        int   contained;
        /** whether or not the track is from a primary particle */
        int   primary;
        /** track length */
        double length;

        /** number of values used in stepInfo */
        int   stepN;
        /** values for the step points */
        double stepInfo[SIZE_STEP * MAX_STEPS];

        /** number of values used in trueInfo */
        int   trueN;
        /** values for the truehit */
        double trueInfo[SIZE_TRUE];

        /** number of values used in edepInfo */
        int edepN;
        /** values for the energy deposition */
        double edepInfo[SIZE_EDEP * MAX_EDEP];

        /** number of values used in simhitInfo */
        int simhitN;
        /** values for the simhits */
        double simhitInfo[SIZE_HITS * MAX_HITS];
      } m_info; /**< object to store all variables */
    };
  }
} //Belle2 namespace
#endif // VXD_SIMULATION_DEBUGFILE_H
