/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOUSCHEKREADERSAD_H_
#define TOUSCHEKREADERSAD_H_

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
   * The reader reads either a specified number of particles from the file, or all particles.
   */
  class TouschekReaderSAD {

  public:

    //Define exceptions
    /** Exception is thrown if the Touschek file could not be opened. */
    BELLE2_DEFINE_EXCEPTION(TouschekCouldNotOpenFileError, "Could not open file %1% !");

    /**
     * Constructor of the TouschekReaderSAD class.
     * @param transMatrix Pointer to the matrix which transforms the particles from the local Touschek to the global geant4 coordinate system.
     */
    TouschekReaderSAD(TGeoHMatrix* transMatrix);

    /**
     * Destructor.
     */
    ~TouschekReaderSAD();

    /**
     * Opens a root file and prepares it for reading.
     * @param filename The filename of the Touschek SAD root file which should be read.
     */
    void open(const std::string& filename) throw(TouschekCouldNotOpenFileError);

    /**
     * Reads the specified number of particles from the file and stores the result in the given MCParticle graph.
     * The number of the last particle which was read is saved. So the next time this method is called the reading
     * continues at the position where the last particle which was read.
     *
     * @param number The number of particles that should be read from the file.
     * @param sRange The +- range for the s value for which particles are loaded.
     * @param graph Reference to the graph which should be filled with the information from the Touschek file.
     * @param beamEnergy The energy of the beam which produced the Touschek particles.
     * @param pdg The pdg value of the type of particle that is read (e.g. 11 for e-, -11 for e+).
     * @return The number of particles which were read.
     */
    int getParticles(int number, double sRange, double beamEnergy, int pdg, MCParticleGraph &graph);


  protected:

    TGeoHMatrix* m_transMatrix; /**< Transformation matrix from local Touschek to global geant4 space. */
    TFile* m_file;              /**< The input root file. */
    TTree* m_tree;              /**< The input root tree. */
    int m_readEntry;            /**< The number of the next entry that will be read. */

    double m_lostX;             /**< x at lost position [m]. */
    double m_lostY;             /**< y at lost position [m]. */
    double m_lostS;             /**< lost position [m] along ring. 0 is ip. Range goes from -L/2 to L/2, where L is the ring circumference. */
    double m_lostPx;            /**< x momentum at lost position [m]. */
    double m_lostPy;            /**< y momentum at lost position [m]. */
  };

}

#endif /* TOUSCHEKREADERSAD_H_ */
