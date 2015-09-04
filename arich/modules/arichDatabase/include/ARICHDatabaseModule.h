/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ARICHDATABASEMODULE_H
#define ARICHDATABASEMODULE_H

#include <framework/core/Module.h>

// DB structures in arich/dbobjects
#include <arich/dbobjects/ARICHAerogelInfo.h>
#include <arich/dbobjects/ARICHHapdQA.h>

#include <string>
#include <TFile.h>
#include <TTimeStamp.h>
#include <TH1S.h>
#include <TH2F.h>
#include <TGraph.h>
#include <vector>

namespace Belle2 {

  //! ARICH database module.
  /*!
    This module writes/read data to/from database.
  */
  class ARICHDatabaseModule : public Module {

  public:


    //! Constructor.
    ARICHDatabaseModule();

    //! Destructor.
    ~ARICHDatabaseModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    void initialize();

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun();

    /**
     * Event processor.
     *
     * Convert reads information from Database and writes tree file.
     */
    void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    void endRun();

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate();

    /**
     *Prints module parameters.
     */
    void printModuleParams() const;

  private:

    int m_runLow;
    int m_runHigh;
    std::string m_wr;
    std::vector<std::string> m_inputFileNames;

    GearDir m_content;
    double m_version;
    std::string m_serial;
    std::string m_id;
    double m_index;
    double m_trlen;
    double m_thickness;
    std::vector<int> m_lambda;
    std::vector<double> m_transmittance;
    int lambda;
    double val;

    std::string m_hapdSerial;
    TTimeStamp m_measurementDate;
    std::vector<TGraph*> m_leakCurrent;
    TH2F* m_hitData2D;
    std::vector<TGraph*> m_noise;
    std::vector<TH1S*> m_hitCount;

    std::string chipname;
    std::string number;
    std::string graphName;
  };

} // Belle2 namespace
#endif
