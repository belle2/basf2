/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOUSCHEKREADERTURTLE_H
#define TOUSCHEKREADERTURTLE_H

#include <framework/core/FrameworkExceptions.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <TGeoMatrix.h>

#include <string>
#include <fstream>

namespace Belle2 {

  /**
   * Class to read Touschek files and store their content in a MCParticle graph.
   *
   * The reader supports retrieving the Touschek information from an ascii text file.
   *
   * The file contains just a list of particles, defined at plane +-50cm from the IP.
   * The reader reads either a specified number of particles from the file, or all
   * particles.
   */
  class TouschekReaderTURTLE {

  public:

    //Define exceptions
    /** Exception is thrown if the Touschek file could not be opened. */
    BELLE2_DEFINE_EXCEPTION(TouschekCouldNotOpenFileError, "Could not open file %1% !");
    /** Exception is thrown if a field in the Touschek file could not be converted to a number. */
    BELLE2_DEFINE_EXCEPTION(TouschekConvertFieldError, "Line %1%: Could not convert field %2%: %3%");

    /**
     * Constructor of the TouschekReader class.
     * @param transMatrix Pointer to the matrix which transforms the particles from the local Touschek to the global geant4 coordinate system.
     * @param pdg The pdg value of the type of particle that is read (e.g. 11 for e-, -11 for e+).
     */
    TouschekReaderTURTLE(TGeoHMatrix* transMatrix, int pdg);

    /**
     * Destructor.
     */
    ~TouschekReaderTURTLE();

    /**
     * Opens an ascii file and prepares it for reading.
     * @param filename The filename of the Touschek ascii file which should be read.
     */
    void open(const std::string& filename);

    /**
     * Reads the specified number of particles from the file and stores the result in the given MCParticle graph.
     * The number of the last particle which was read is saved. So the next time this method is called the reading
     * continues at the position where the last particle which was read.
     *
     * @param number The number of particles that should be read from the file. Set it to -1 to read all particles.
     * @param graph Reference to the graph which should be filled with the information from the Touschek file.
     * @return The number of particles which were read.
     */
    int getParticles(int number, MCParticleGraph& graph);


  protected:

    TGeoHMatrix* m_transMatrix; /**< Transformation matrix from local Touschek to global geant4 space. */
    int m_pdg;                  /**< The pdg value of the type of particle that is read (e.g. 11 for e-, -11 for e+).*/
    std::ifstream m_input;      /**< The input stream of the ascii file. */
    int m_lineNum;              /**< The line number in the ascii file of the last particle which was read. */
  };

}

#endif /* TOUSCHEKREADERTURTLE_H */
