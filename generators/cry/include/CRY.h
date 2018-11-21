/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CRY_H
#define CRY_H

#include "cry/CRYGenerator.h"
#include "cry/CRYSetup.h"

#include <mdst/dataobjects/MCParticleGraph.h>
#include <utility>
#include <TRandom3.h>

namespace Belle2 {
  /**
   *Minimal class for external random generator to be used in CRY.
   */
  class CRYRndm {
  public:
    // Constructor
    CRYRndm();
    double rndm(); /**< flat random generator. */

  protected:

  private:

  };

  /**
   *Wrapper for RNG to be used with framework generator.
   */
  template<class T> class RNGWrapper {
  public:
    /** set object. */
    static void set(T* object, double (T::*func)(void));
    /** random generator. */
    static double rng(void);
  private:
    static T* m_obj;  /**< directory that holds cosmic data files. */
    static double (T::*m_func)(void);  /**< directory that holds cosmic data files. */
  };// end of RNGWrapper class

  template<class T> T* RNGWrapper<T>::m_obj; /**< RNGWrapper. */

  template<class T> double (T::*RNGWrapper<T>::m_func)(void); /**< RNGWrapper. */

  /** RNGWrapper. */
  template<class T> void RNGWrapper<T>::set(T* object, double (T::*func)(void))
  {
    m_obj = object; m_func = func;
  }

  /** RNGWrapper. */
  template<class T> double RNGWrapper<T>::rng(void) { return (m_obj->*m_func)(); }
}

namespace Belle2 {
  /**
   * C++ Interface for the generator CRY.
   *
   */

  class CRY {
  public:

    /** Constructor.
     * Sets the default settings.
     */
    CRY();

    /** Destructor. */
    ~CRY();

    /** Sets the default settings for the CRY generator. */
    void setDefaultSettings();

    /**
     * Initializes the generator.
     */
    void init();

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     */
    void generateEvent(MCParticleGraph& mcGraph);

    /** Sets the directory that holds cosmic data.
     * @param cosmicdatadir directory that holds the data.
     */
    void setCosmicDataDir(const std::string& cosmicdatadir) { m_cosmicDataDir = cosmicdatadir; }

    /** Sets the setup file.
     * @param setupfile setup file.
     */
    void setSetupFile(const std::string& setupfile) { m_setupFile = setupfile; }

    /** Sets the height of the accept box.
     * @param acceptheight height of the accept box.
     */
    void setAcceptHeight(double acceptheight) { m_acceptHeight = acceptheight; }

    /** Sets the length of the accept box.
     * @param acceptlength length of the accept box.
     */
    void setAcceptLength(double acceptlength) { m_acceptLength = acceptlength; }

    /** Sets the width of the accept box.
     * @param acceptwidth width of the accept box.
     */
    void setAcceptWidth(double acceptwidth) { m_acceptWidth = acceptwidth; }

    /** Sets the height of the keep box.
     * @param keepheight height of the keep box.
     */
    void setKeepHeight(double keepheight) { m_keepHeight = keepheight; }

    /** Sets the length of the keep box.
     * @param keeplength length of the keep box.
     */
    void setKeepLength(double keeplength) { m_keepLength = keeplength; }

    /** Sets the width of the keep box.
     * @param keepwidth width of the keep box.
     */
    void setKeepWidth(double keepwidth) { m_keepWidth = keepwidth; }

    /** Sets the time offset.
     * @param timeoffset time offset for particles starting at the world box.
     */
    void setTimeOffset(double timeoffset) { m_timeOffset = timeoffset; }

    /** Sets the kinetic energy threshold, particles below that value will be ignored.
     * @param kineticenergythreshold kinetic energy threshold.
     */
    void setKineticEnergyThreshold(double kineticenergythreshold) { m_kineticEnergyThreshold = kineticenergythreshold; }

    /** Sets the maximum number of trials.
     * @param maxtrials maximum number of trials.
     */
    void setMaxTrials(int maxtrials) { m_maxTrials = maxtrials; }

    /**
     * Terminates the generator.
     * Closes the internal generator.
     */
    void term();

  protected:
    std::string m_cosmicDataDir;  /**< directory that holds cosmic data files. */
    std::string m_setupFile;   /**< setupfile with uder input. */

    double m_acceptLength;  /**< length of a box used to reject quickly non detector particles. */
    double m_acceptWidth;  /**< width of a box used to reject quickly non detector particles. */
    double m_acceptHeight;  /**< height of a box used to reject quickly non detector particles. */
    double m_keepLength;  /**< length of a box used to keep particles. */
    double m_keepWidth;  /**< width of a box used to keep particles. */
    double m_keepHeight;  /**< height of a box used to keep particles. */
    double m_timeOffset;  /**< time offset in seconds. */

    double m_kineticEnergyThreshold;  /**< kinetic energy threshold. */

    int m_maxTrials;   /**< number of trials per event. */
    int m_maxTrialsRun;   /**< maximum number of trials per event for the full run (to check possible problems). */
    int m_totalTrials; /**< total number of thrown events. */

    bool m_checkAcceptance; /**< if acceptance box is given, perform check. */
    bool m_checkKeep; /**< if keep box is given, perform check. */

    TVector3 m_B1; /**< coordinate edges  of acceptance box. */
    TVector3 m_B2; /**< coordinate edges  of acceptance box. */
    TVector3 m_B1keep; /**< coordinate edges of keep box. */
    TVector3 m_B2keep; /**< coordinate edges  of keep box. */

    CRYSetup* m_crySetup;   /**< The CRY generator setup. */
    CRYGenerator* m_cryGenerator;   /**< The CRY generator. */
    CRYRndm* m_cryRNG;   /**< Random number generator. */

    /** Apply the settings to the internal generator. */
    void applySettings();

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param pdg The PDG code of the particle.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, const double* vertex, const double ptime, const int pdg,
                       bool isVirtual = false, bool isInitial = false);

    double m_xlow;  /**< lower x coordinate of the top volume. */
    double m_xhigh;  /**< upper x coordinate of the top volume. */
    double m_ylow;  /**< lower y coordinate of the top volume. */
    double m_yhigh;  /**< upper y coordinate of the top volume. */
    double m_zlow;  /**< lower z coordinate of the top volume. */
    double m_zhigh;  /**< upper z coordinate of the top volume. */
    double m_startTime;  /**< start time. */

    double m_newvtx[3];  /**< vertizes projected to bounding box. */

  private:
    void FillTopVolumeCoordinates();  /**< Get top volume from geometry and store its boundary coordinates. */
    void ProjectToTopVolume(const double vtx[], const double p[], double newvtx[]);  /**< Project particle to the top volume box. */
    bool InBox(TVector3 hit, TVector3 B1, TVector3 B2, int axis); /**< check if line intersects with box. */
    bool GetIntersection(float fDst1, float fDst2, TVector3 P1, TVector3 P2, TVector3& hit);  /**< get the intersection coorindates. */
    bool CheckLineBox(TVector3 B1, TVector3 B2, TVector3 L1, TVector3 L2,
                      TVector3& hit);  /**< main function to check line box interesection. */

  };
} // end namespace Belle2
#endif /* CRY_H */
