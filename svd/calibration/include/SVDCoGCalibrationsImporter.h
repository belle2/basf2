/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  class SVDCoGCalibrationsImporter {

  public:

    /**
     * Default constructor
     */
    SVDCoGCalibrationsImporter():
      m_firstExperiment(0), m_firstRun(0),
      m_lastExperiment(-1), m_lastRun(-1)
    {
    }

    /**
     * A constructor accepting as arguments the interval of
     * validity coordinates.
     */
    SVDCoGCalibrationsImporter(int fexp, int frun, int lexp, int lrun):
      m_firstExperiment(fexp), m_firstRun(frun),
      m_lastExperiment(lexp), m_lastRun(lrun)
    {
    }

    virtual ~SVDCoGCalibrationsImporter()
    {}

    /** Function returning the index used for Histos */
    int indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber);

    /** Function returning "Internal" or "External" depending on the index */
    TString IntExtFromIndex(int idx);

    /** Function returning "Forward" or "Backword" depending on the index */
    TString FWFromIndex(int idx);

    /** Function to fill the DB with Correction3 values */
    void importSVDTimeShiftCorrections(bool m_fillAreas, TString filename);

  private:

    int m_firstExperiment; /**< First experiment. */
    int m_firstRun; /**< First run. */
    int m_lastExperiment; /**< Last experiment */
    int m_lastRun; /**< Last run. */
    ClassDef(SVDCoGCalibrationsImporter, 1);
  };

} // Belle2 namespace
