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

#include <string>
#include <fstream>

#include <Math/LorentzRotation.h>

#include "HepMC/GenEvent.h"

namespace Belle2 {

  /**
   * Class to read HepMC files and store the content in a MCParticle graph.
   *
   * The reader supports retrieving the HepMC2 information from an ascii text file as provided by for example pythia8.
   *
   * the events are read sequentially with the option of skipping a certain number of
   * events. Random access of events is not possible.
   */
  class HepMCReader {

  public:

    //Define exceptions
    /** Exception is thrown if the HepMC file could not be opened. */
    BELLE2_DEFINE_EXCEPTION(HepMCCouldNotOpenFileError, "Could not open file %1% !");
    /** Exception is thrown if the given indices of the daughters are not valid. */
    BELLE2_DEFINE_EXCEPTION(HepMCInvalidDaughterIndicesError,
                            "Event %1%: Invalid daughter indices d1=%2%, d2=%3%, N=%4% (0<=d1<=d2<=N required)");
    /** Exception is thrown if the header specifying the event header could not be parsed. */
    BELLE2_DEFINE_EXCEPTION(HepMCHeaderNotValidError, "Event %1%: Event header not understood: %2%");
    /** Exception is thrown if a field in the HepMC file could not be converted to a number. */
    BELLE2_DEFINE_EXCEPTION(HepMCConvertFieldError, "Event %1%: Could not convert field %2%: %3%");
    /** Exception is thrown if the format of a line of the HepMC file could not be parsed. */
    BELLE2_DEFINE_EXCEPTION(HepMCParticleFormatError, "Event %1%: Particle format not understood, got %2% fields !");
    /** Exception is thrown if the format of a line of the HepMC file could not be parsed. */
    BELLE2_DEFINE_EXCEPTION(HepMCInvalidEventError, "Event is invalid.");
    /** Exception is thrown if the number of particles for this event is 0 or less.  */
    BELLE2_DEFINE_EXCEPTION(HepMCEmptyEventError,
                            "Event %1%: Number of particles in event is %2% ! (This could mean EOF is reached.) ");
    /**
     * Constructor.
     */
    HepMCReader(const int minEvent = 0, const int maxEvent = INT_MAX): m_nVirtual(0), m_wrongSignPz(false), m_minEvent(minEvent),
      m_maxEvent(maxEvent) {}

    /**
     * Destructor.
     */
    ~HepMCReader() { if (m_input) {m_input.close();} }

    /**
     * Opens an ascii file and prepares it for reading.
     * @param filename The filename of the HepMC2 ascii file which should be read.
     */
    void open(const std::string& filename);

    /**
     * Closes the current input file to allow opening the next one.
     */
    void closeCurrentInputFile() {m_input.close();}

    /**
     * Got to next event in bounds.
     * Throw InvalidEventException if we went over the bounds
     * */
    int nextValidEvent(HepMC::GenEvent& evt);
    /**
     * Reads the next event and stores the result in the given MCParticle graph.
     * @param graph Reference to the graph which should be filled with the information from the Hepevt file.
     * @param weight Reference to the event weight which can be filled from the file.
     * @return event numer if the event could be read and the number was provided in the file.
     */
    int getEvent(MCParticleGraph& graph, double& weight);

    int m_nVirtual;        /**< The number of particles in each event with a set Virtual flag. */
    bool m_wrongSignPz;    /**< Bool to indicate that HER and LER were swapped. */
    const int m_minEvent; /**< min event nr to process */
    const int m_maxEvent; /**< max events to process */
    ROOT::Math::LorentzRotation m_labboost;     /**< Boost&rotation vector for boost from CM to LAB. */

    int countEvents(const std::string& filename); /**< Count events in file */
  protected:

    void readNextEvent(HepMC::GenEvent& evt); /**< read the next event from the IO stream and write into evt */
    std::ifstream m_input; /**< The input stream of the ascii file. */

  };

}

