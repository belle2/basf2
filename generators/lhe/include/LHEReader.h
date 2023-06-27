/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/FrameworkExceptions.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <boost/tokenizer.hpp>

#include <string>
#include <fstream>

namespace Belle2 {

  /**
   * Class to read LHE files and store the content in a MCParticle graph.
   * The class is very similar to the HepEvt Reader
   *
   * The events are read sequentially with the option of skipping a certain number of
   * events. Random access of events is not possible.
   */
  class LHEReader {

  public:

    //Define exceptions
    /** Exception is thrown if the LHE file could not be opened. */
    BELLE2_DEFINE_EXCEPTION(LHECouldNotOpenFileError, "Could not open file %1% !");
    /** Exception is thrown if the given indices of the daughters are not valid. */
    BELLE2_DEFINE_EXCEPTION(LHEInvalidDaughterIndicesError,
                            "Line %1%: Invalid daughter indices d1=%2%, d2=%3%, N=%4% (0<=d1<=d2<=N required)");
    /** Exception is thrown if the header specifying the event header could not be parsed. */
    BELLE2_DEFINE_EXCEPTION(LHEHeaderNotValidError, "Line %1%: Event header not understood: %2%");
    /** Exception is thrown if a field in the LHE file could not be converted to a number. */
    BELLE2_DEFINE_EXCEPTION(LHEConvertFieldError, "Line %1%: Could not convert field %2%: %3%");
    /** Exception is thrown if the format of a line of the LHE file could not be parsed. */
    BELLE2_DEFINE_EXCEPTION(LHEParticleFormatError, "Line %1%: Particle format not understood, got %2% fields !");
    /** Exception is thrown if the number of particles for this event is 0 or less.  */
    BELLE2_DEFINE_EXCEPTION(LHEEmptyEventError, "Line %1%: Number of particles in event is %2% ! (This could mean EOF is reached.) ");
    /**
     * Constructor.
     */
    LHEReader():
      m_wrongSignPz(false),
      m_lineNr(0),
      m_indexInitial(0),
      m_indexVirtual(0)
    {}

    /**
     * Destructor.
     */
    ~LHEReader() { if (m_input) m_input.close(); }

    /**
     * Opens an ascii file and prepares it for reading.
     * @param filename The filename of the LHE ascii file which should be read.
     */
    void open(const std::string& filename);

    /**
     * Closes the current input file to allow opening the next one.
     */
    void closeCurrentInputFile() {m_input.close();}

    /**
     * Reads the next event and stores the result in the given MCParticle graph.
     * @param graph Reference to the graph which should be filled with the information from the LHE file.
     * @param weight Reference to the event weight which can be filled from the file.
     * @return event numer if the event could be read and the number was provided in the file.
     */
    int getEvent(MCParticleGraph& graph, double& weight);

    /**
     * Skips a given number of events.
     * @param n The number of events which should be skipped.
     * @return True if the events could be skipped.
     */
    bool skipEvents(int n);

    /**
     * Set the maximum index of particles in each event that must be set as c_Initial (1-based).
     * @param[in] index Maximum index for c_Initial.
     */
    void setInitialIndex(int index) { m_indexInitial = index; }

    /**
     * Set the maximum index of particles in each event that must be set as c_IsVirtual (1-based).
     * @param[in] index Maximum index for c_IsVirtual.
     */
    void setVirtualIndex(int index) { m_indexVirtual = index; }

    bool m_wrongSignPz;    /**< Bool to indicate that HER and LER were swapped. */
    double m_meanDecayLength = 0.;        /**< Mean lifetime*c of displaced particle. */
    double m_Rmin = 0.; /**< Minimum  of vertex distance to IP.*/
    double m_Rmax = 0.; /**< Maximum of vertex distance to IP.*/
    int m_pdgDisplaced = 0; /**< PDG code of the displaced particle being studied*/

  protected:
    /** Just a typedef for simple use of the boost::tokenizer to split the lines */
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    /** The characters at which to split, defaults to ",; \t" */
    static const boost::char_separator<char> sep;

    int m_lineNr;          /**< The current line number within the ascii file. */
    std::ifstream m_input; /**< The input stream of the ascii file. */

    //  /*   int eventID; /**< The event ID number if provided in LHEfile else -1.  */ */
    ///*     double eventWeight; /**< The event weight if provided in LHEfile else 1.  */ */


    /**
     * Returns the current line from the LHE ascii file.
     * @return The current line as a string.
     */
    std::string getLine();

    /**
     * Reads the event header from the hepevt file.
     * @return The number of particles for the current event.
     * @param eventWeight reference to the eventWeight which can both be read from the file.
     */
    int readEventHeader(double& eventWeight);

    /**
     * Reads the information for a single particle from the LHE file.
     * @param particle Reference to the particle which will be filled with the information from the LHE file.
     */
    int readParticle(MCParticleGraph::GraphParticle& particle);

    int m_indexInitial;  /**< Maximum index of particles in each event that must be set as c_Initial (1-based). */

    int m_indexVirtual;  /**< Maximum index of particles in each event that must be set as c_IsVirtual (1-based). */

  };

}
