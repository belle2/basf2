/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HEPEVTREADER_H
#define HEPEVTREADER_H

#include <framework/core/FrameworkExceptions.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <boost/tokenizer.hpp>

#include <string>
#include <fstream>

#include <TLorentzRotation.h>

namespace Belle2 {

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

    //Define exceptions
    /** Exception is thrown if the HepEvt file could not be opened. */
    BELLE2_DEFINE_EXCEPTION(HepEvtCouldNotOpenFileError, "Could not open file %1% !");
    /** Exception is thrown if the given indices of the daughters are not valid. */
    BELLE2_DEFINE_EXCEPTION(HepEvtInvalidDaughterIndicesError,
                            "Line %1%: Invalid daughter indices d1=%2%, d2=%3%, N=%4% (0<=d1<=d2<=N required)");
    /** Exception is thrown if the header specifying the event header could not be parsed. */
    BELLE2_DEFINE_EXCEPTION(HepEvtHeaderNotValidError, "Line %1%: Event header not understood: %2%");
    /** Exception is thrown if a field in the HepEvt file could not be converted to a number. */
    BELLE2_DEFINE_EXCEPTION(HepEvtConvertFieldError, "Line %1%: Could not convert field %2%: %3%");
    /** Exception is thrown if the format of a line of the HepEvt file could not be parsed. */
    BELLE2_DEFINE_EXCEPTION(HepEvtParticleFormatError, "Line %1%: Particle format not understood, got %2% fields !");
    /** Exception is thrown if the number of particles for this event is 0 or less.  */
    BELLE2_DEFINE_EXCEPTION(HepEvtEmptyEventError,
                            "Line %1%: Number of particles in event is %2% ! (This could mean EOF is reached.) ");
    /**
     * Constructor.
     */
    HepevtReader(): m_nVirtual(0), m_wrongSignPz(false), m_lineNr(0) {}

    /**
     * Destructor.
     */
    ~HepevtReader() { if (m_input) m_input.close(); }

    /**
     * Opens an ascii file and prepares it for reading.
     * @param filename The filename of the Hepevt ascii file which should be read.
     */
    void open(const std::string& filename);

    /**
     * Closes the current input file to allow opening the next one.
     */
    void closeCurrentInputFile() {m_input.close();}

    /**
     * Reads the next event and stores the result in the given MCParticle graph.
     * @param graph Reference to the graph which should be filled with the information from the Hepevt file and the Reference to the event weight which can be filled from the file.
     * @return event numer if the event could be read and the number was provided in the file.
     */
    int getEvent(MCParticleGraph& graph, double& weight);

    /**
     * Skips a given number of events.
     * @param n The number of events which should be skipped.
     * @return True if the events could be skipped.
     */
    bool skipEvents(int n);

    int m_nVirtual;        /**< The number of particles in each event with a set Virtual flag. */
    bool m_wrongSignPz;    /**< Bool to indicate that HER and LER were swapped. */
    TLorentzRotation m_labboost;     /**< Boost&rotation vector for boost from CM to LAB. */


  protected:
    /** Just a typedef for simple use of the boost::tokenizer to split the lines */
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    /** The characters at which to split, defaults to ",; \t" */
    static const boost::char_separator<char> sep;

    int m_lineNr;          /**< The current line number within the ascii file. */
    std::ifstream m_input; /**< The input stream of the ascii file. */

    //  /*   int eventID; /**< The event ID number if provided in HepEvtfile else -1.  */ */
    ///*     double eventWeight; /**< The event weight if provided in HepEvtfile else 1.  */ */


    /**
     * Returns the current line from the Hepevt ascii file.
     * @return The current line as a string.
     */
    std::string getLine();

    /**
     * Reads the event header from the hepevt file.
     * @return The number of particles for the current event.
     * @params: References to the eventID and the eventWeight which can both be read from the file.
     */
    int readEventHeader(int& eventID, double& eventWeight);

    /**
     * Reads the information for a single particle from the Hepevt file.
     * @param particle Reference to the particle which will be filled with the information from the Hepevt file.
     */
    void readParticle(MCParticleGraph::GraphParticle& particle);
  };

}

#endif //HEPEVTREADER_H
