/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama, Igal Jaegle             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef READERSAD_H
#define READERSAD_H

#include <framework/core/FrameworkExceptions.h>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <TGeoMatrix.h>
#include <TFile.h>
#include <TTree.h>

#include <string>


namespace Belle2 {

  /**
   * Class to read files that have been created by SAD and store their content in a MCParticle graph.
   *
   * The input data is stored in a root file and contains the particles together with their lost rate.
   * The reader reads one particle from the file, calculates the number of 'real' particles and
   * creates a new event for each of them.
   */
  class ReaderSAD {

  public:

    //Define exceptions
    /** Exception is thrown if the SAD file could not be opened. */
    BELLE2_DEFINE_EXCEPTION(SADCouldNotOpenFileError, "Could not open file %1% !");
    /** Exception is thrown if the end of the SAD file has been reached. */
    BELLE2_DEFINE_EXCEPTION(SADEndOfFile, "End of the SAD file.");

    /** The both accelerator rings. */
    enum AcceleratorRings {
      c_HER = 0, /**< High Energy Ring (electrons) */
      c_LER = 1  /**< Low Energy Ring (positrons) */
    };

    /**
     * Constructor of the ReaderSAD class.
     */
    ReaderSAD();

    /**
     * Destructor.
     */
    ~ReaderSAD();

    /**
     * Initializes the reader, sets the particle parameters and calculates important values.
     * @param transMatrix Pointer to the matrix which transforms the particles from the local SAD to the global geant4 coordinate system.
     * @param sRange The +- range for the s value for which particles are loaded.
     * @param accRing The accelerator ring from which the particles originate.
     * @param readoutTime The readout time of the detector in [ns].
     */
    void initialize(TGeoHMatrix* transMatrix, double sRange, AcceleratorRings accRing, double readoutTime);

    /**
     * Opens a root file and prepares it for reading.
     * @param filename The filename of the SAD root file which should be read.
     */
    void open(const std::string& filename);

    /**
     * Sets the resolution of the momentum for the real particles.
     * Allows each real particle to be smeared according to the specified momentum resolution.
     * @param pxRes The resolution for the x momentum component.
     * @param pyRes The resolution for the y momentum component.
     */
    void setMomentumRes(double pxRes, double pyRes) { m_pxRes = pxRes; m_pyRes = pyRes; }

    /**
     * Reads one SAD particle from the file and creates one event per SAD particle.
     * The loss rate of the SAD particle is stored in the weight attribute of the event meta info.
     *
     * @param graph Reference to the graph which should be filled with the information from the SAD file.
     * @return The loss rate of the SAD particle which was read. Returns -1 if an error occurred.
     */
    double getSADParticle(MCParticleGraph& graph);

    /**
     * Reads one SAD particle from the file, calculates the number of real particles which are represented by the SAD particle
     * and creates one event per real particle.
     *
     * @param graph Reference to the graph which should be filled with the information from the SAD file.
     * @return True if the particle could be read.
     */
    bool getRealParticle(MCParticleGraph& graph);

    /**
     * Reads all SAD particles from the file into the MCParticles collection which are inside the specified s range.
     *
     * @param graph Reference to the graph which should be filled with the information from the SAD file.
     */
    void addAllSADParticles(MCParticleGraph& graph);

    /**
     * Transformation matrix.
     *
     * @param accRing The accelerator ring from which the particles originate.
     * @param s s value.
     */
    TGeoHMatrix SADtoGeant(ReaderSAD::AcceleratorRings accRing, double s);


  protected:

    TFile* m_file;                 /**< The input root file. */
    TTree* m_tree;                 /**< The input root tree. */

    TGeoHMatrix* m_transMatrix;    /**< Transformation matrix from local SAD to global geant4 space. */
    double m_sRange;               /**< The +- range for the s value for which particles are loaded. */
    AcceleratorRings m_accRing;    /**< The accelerator ring from which the particles originate. */
    double m_pxRes;                /**< The resolution for the x momentum component of the SAD real particle. */
    double m_pyRes;                /**< The resolution for the y momentum component of the SAD real particle. */

    double m_SADToRealFactor;      /**< The factor to calculate the number of real particles from a SAD particle. */
    double m_readoutTime;          /**< The readout time. */

    unsigned int m_realPartNum;    /**< The current number of the created real particles. */
    unsigned int m_realPartEntry;  /**< The current number of the created real particles. */
    int m_readEntry;               /**< The current number of the SAD entry that is read. */

    double m_lostX;             /**< x at lost position [m]. */
    double m_lostY;             /**< y at lost position [m]. */
    double m_lostS;             /**< lost position [m] along ring. 0 is the IP. Range goes from -L/2 to L/2, where L is the ring circumference. */
    double m_lostPx;            /**< x momentum at lost position [m]. */
    double m_lostPy;            /**< y momentum at lost position [m]. */
    double m_lostRate;          /**< loss rate [Hz]> */
    double m_lostE;             /**< energy at lost position [m]. */

    double m_inputSAD_ssraw;     /**< scattered position [m]*/
    double m_inputSAD_sraw;      /**< lost position [m*/
    double m_inputSAD_ss;        /**< scattered position (|s|<Ltot/2) [m] */
    double m_inputSAD_s;         /**< lost position (|s|<Ltot/2) [m] */
    double m_inputSAD_Lss;       /**< length of element in which scattered [m] */
    int m_inputSAD_nturn;     /**< number of turns from scattered to lost */
    double m_inputSAD_x;         /**< x at lost position [m]. */
    double m_inputSAD_y;         /**< y at lost position [m]. */
    double m_inputSAD_px;        /**< x momentum at lost position [m]. */
    double m_inputSAD_py;        /**< y momentum at lost position [m]. */
    double m_inputSAD_xraw;      /**< x at lost position [m] before matching onto beam pipe inner surface */
    double m_inputSAD_yraw;      /**< y at lost position [m] before matching onto beam pipe inner surface */
    double m_inputSAD_r;         /**< sqrt(x*x+y*y) [m] */
    double m_inputSAD_rr;        /**< sqrt(x*x+y*y) [m] before matching onto beam pipe inner surface*/
    double m_inputSAD_dp_over_p0;/**<  dp_over_p0 */
    double m_inputSAD_E;         /**< energy at lost position [m]. */
    double m_inputSAD_rate;      /**< loss rate [Hz] */
    double m_inputSAD_watt;      /**< loss wattage [W] */

  private:

    /** Convert the parameters from the SAD units to the basf2 units. */
    void convertParamsToSADUnits();

    /** Adds the current particle described by the member variables to the MCParticles collection.
     *
     * @param graph Reference to the graph which should be filled with the information from the SAD file.
     * @param gaussSmearing If set to true the particle momentum is smeared using a Gaussian.
     */
    void addParticleToMCParticles(MCParticleGraph& graph, bool gaussSmearing = false);

    /**
     * Calculates the number of real particles for a SAD particle.
     * @param rate The loss rate of the SAD particle.
     * @return The number of real particles for the given loss rate.
     */
    int calculateRealParticleNumber(double rate);

    /** Calculates the transformation matrix from local SAD to global geant4 space. */
//    TGeoHMatrix SADtoGeant(ReaderSAD::AcceleratorRings accRing, double s);

    struct straightElement {
      double x0;        /**< Initial position in X */
      double z0;        /**< Initial position in Z */
      double l;         /**< Parameter */
      double phi;       /**< Phi angle */
    };

    /** Sensitive Element  */
    struct bendingElement {
      double rt;        /**< Parameter */
      double x0;        /**< Initial position in X  */
      double z0;        /**< Initial position in Z  */
      double sphi;      /**< Bending Angle */
      double dphi;      /**< Bending Angle for torus in phi */
    };

  };

}

#endif /* READERSAD_H */
