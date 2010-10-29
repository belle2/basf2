/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HEPEVTREADER_H
#define HEPEVTREADER_H

#include <string>
#include <fstream>

#include <framework/core/FrameworkExceptions.h>
#include <generators/objects/MCParticleGraph.h>

namespace Belle2 {

  //Define exceptions
  BELLE2_DEFINE_EXCEPTION(HepEvtCouldNotOpenFileError, "Could not open file %1% !");
  BELLE2_DEFINE_EXCEPTION(HepEvtInvalidDaughterIndicesError, "Line %1%: Invalid daughter indices d1=%2%, d2=%3%, N=%4% (0<=d1<=d2<=N required)");
  BELLE2_DEFINE_EXCEPTION(HepEvtHeaderNotValidError, "Line %1%: Event header not understood: %2%");
  BELLE2_DEFINE_EXCEPTION(HepEvtConvertFieldError, "Line %1%: Could not convert field %2%: %3%");
  BELLE2_DEFINE_EXCEPTION(HepEvtParticleFormatError, "Line %1%: Particle format not understood, got %2% fields !");

  /**
   * Class to read Hepevt files and store the content in a MCParticle graph.
   *
   * The reader supports retrieving the Hepevt information from an ascii text file.
   * Both formats, the short and the long LUND 7 format are supported.
   *
   * The events are read sequentially with the option of skipping a certain number of
   * events. Random access of events is not possible.
   */
  class HepevtReader {

  public:

    /**
     * Constructor.
     */
    HepevtReader() {}

    /**
     * Destructor.
     */
    ~HepevtReader() { if (m_input) m_input.close(); }

    /**
     * Opens an ascii file and prepares it for reading.
     * @param filename The filename of the Hepevt ascii file which should be read.
     */
    void open(const std::string& filename) throw(HepEvtCouldNotOpenFileError);

    /**
     * Reads the next event and stores the result in the given MCParticle graph.
     * @param graph Reference to the graph which should be filled with the information from the Hepevt file.
     * @return True if the event could be read.
     */
    bool getEvent(MCParticleGraph &graph) throw(HepEvtInvalidDaughterIndicesError);

    /**
     * Skips a given number of events.
     * @param n The number of events which should be skipped.
     * @return True if the events could be skipped.
     */
    bool skipEvents(int n);


  protected:

    int m_lineNr;          /**< The current line number within the ascii file. */
    std::ifstream m_input; /**< The input stream of the ascii file. */

    /**
     * Returns the current line from the Hepevt ascii file.
     * @return The current line as a string.
     */
    std::string getLine();

    /**
     * Reads the event header from the hepevt file.
     * @return The number of particles for the current event.
     */
    int readEventHeader() throw(HepEvtHeaderNotValidError);

    /**
     * Reads the information for a single particle from the Hepevt file.
     * @param particle Reference to the particle which will be filled with the information from the Hepevt file.
     */
    void readParticle(MCParticleGraph::GraphParticle &particle) throw(HepEvtConvertFieldError, HepEvtParticleFormatError);
  };

}

#endif //HEPEVTREADER_H
