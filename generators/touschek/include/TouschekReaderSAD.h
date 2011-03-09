/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOUSCHEKREADERSAD_H
#define TOUSCHEKREADERSAD_H

#include <framework/core/FrameworkExceptions.h>

#include <generators/dataobjects/MCParticleGraph.h>

#include <TGeoMatrix.h>
#include <TFile.h>
#include <TTree.h>

#include <string>


namespace Belle2 {

  /**
   * Class to read Touschek files that were created by SAD and store their content in a MCParticle graph.
   *
   * The data is stored in a root file and contains only the particles lost by the Touschek effect.
   * The reader reads one particle from the file, calculates the number of real Touschek particles and
   * creates a new event for each of them.
   */
  class TouschekReaderSAD {

  public:

    //Define exceptions
    /** Exception is thrown if the Touschek file could not be opened. */
    BELLE2_DEFINE_EXCEPTION(TouschekCouldNotOpenFileError, "Could not open file %1% !");
    /** Exception is thrown if the end of the Touschek SAD file was reached. */
    BELLE2_DEFINE_EXCEPTION(TouschekEndOfFile, "End of Touschek SAD file.");

    /**
     * Constructor of the TouschekReaderSAD class.
     */
    TouschekReaderSAD();

    /**
     * Destructor.
     */
    ~TouschekReaderSAD();

    /**
     * Initializes the reader, sets the beam parameters and calculates important values.
     * @param transMatrix Pointer to the matrix which transforms the particles from the local Touschek to the global geant4 coordinate system.
     * @param sRange The +- range for the s value for which particles are loaded.
     * @param pdg The pdg code of the Touschek particles.
     * @param beamEnergy The beam energy in [GeV].
     * @param current The current of the beam in Ampere.
     * @param lifetime The Touschek beam lifetime in [ns].
     * @param readoutTime The readout time of the detector in [ns].
     */
    void initialize(TGeoHMatrix* transMatrix, double sRange, int pdg, double beamEnergy, double current, double lifetime, double readoutTime);

    /**
     * Opens a root file and prepares it for reading.
     * @param filename The filename of the Touschek SAD root file which should be read.
     */
    void open(const std::string& filename) throw(TouschekCouldNotOpenFileError);

    /**
     * Reads one SAD particle from the file, calculates the number of real particles which are represented by the SAD particle
     * and creates one event per real particle.
     *
     * @param graph Reference to the graph which should be filled with the information from the Touschek file.
     * @return The weight of the SAD particle which was read. Returns -1 if an error occured.
     */
    double getParticle(MCParticleGraph &graph);


  protected:

    TFile* m_file;                 /**< The input root file. */
    TTree* m_tree;                 /**< The input root tree. */

    TGeoHMatrix* m_transMatrix;    /**< Transformation matrix from local Touschek to global geant4 space. */
    double m_sRange;               /**< The +- range for the s value for which particles are loaded. */
    int m_pdg;                     /**< The pdg code of the Touschek particles. */
    double m_beamenergy;           /**< The beam energy in [GeV]. */

    double m_touschekToRealFactor; /**< The factor to calculate the number of real particles from a SAD Touschek particle. */
    unsigned int m_realPartNum;    /**< The current number of the created real particles. */
    unsigned int m_realPartEntry;  /**< The current number of the created real particles. */
    unsigned int m_readEntry;      /**< The number of the next entry that will be read. */

    double m_lostX;             /**< x at lost position [m]. */
    double m_lostY;             /**< y at lost position [m]. */
    double m_lostS;             /**< lost position [m] along ring. 0 is ip. Range goes from -L/2 to L/2, where L is the ring circumference. */
    double m_lostPx;            /**< x momentum at lost position [m]. */
    double m_lostPy;            /**< y momentum at lost position [m]. */
    double m_lostW;            /**< weight describing lost fraction. */


  private:

    /**
     * Calculates the number of real particles for a Touschek particle.
     * @param weight The weight of the Touschek particle.
     * @return The number of real particles for the given weight.
     */
    int calculateRealParticleNumber(double weight);
  };

}

#endif /* TOUSCHEKREADERSAD_H */
