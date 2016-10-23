/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Leakage corrections for ECL showers (N1).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Alon Hershenhorn (hershen@physics.ubc.ca)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//Root
#include <TObject.h>
#include <TTree.h>
#include <TDirectory.h>

namespace Belle2 {

  /**
   * Corrections to the second moment shower shape.
   * The class takes ownership of m_correction and m_helper (i.e. it moves them to m_directory) so is responsible to delete them.
   */

  class ECLShowerCorrectorLeakageCorrection: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLShowerCorrectorLeakageCorrection(): m_directory(new TDirectory()), m_helper(nullptr), m_correction(nullptr) {};

    /**
     * Constructor
     */
    ECLShowerCorrectorLeakageCorrection(TDirectory* directory, TTree* helper, TTree* correction) :
      m_directory(directory),
      m_helper(helper),
      m_correction(correction)
    {
      B2WARNING("line = " << __LINE__);
      //Create new directory, if needed
///      if( !m_directory) m_directory = new TDirectory();
      B2WARNING("line = " << __LINE__);
      //Move trees to m_directory
//     if(m_helper) m_helper->SetDirectory(m_directory);
      B2WARNING("line = " << __LINE__);
//      if(m_correction) m_correction->SetDirectory(m_directory);
//        B2WARNING( "line = " << __LINE__);
    }

    /**
     * Destructor
     * We need to delete trees, even though we didn't create them because we moved them to m_directory
     */
    ~ECLShowerCorrectorLeakageCorrection()
    {
      delete m_helper;
      delete m_correction;
      delete m_directory;
    }

    /** Return helper ntuple
     * @return ntuple with helper information
     */
    TTree* getHelperTree() const {return m_helper;};

    /** Return correction ntuple
     * @return ntuple with correction information
     */
    TTree* getCorrectionTree() const {return m_correction;};

    /** Set helper ntuple
     * @param helper ntuple
     */
    void setHelperNtuple(TTree* helper)
    {

      //Delete old tree, if it exists
      if (m_helper) delete m_helper;
      m_helper = helper;
      if (m_helper) m_helper->SetDirectory(m_directory);
    }

    /** Set correction ntuple
     * @param correction ntuple
     */
    void setCorrectionNtuple(TTree* correction)
    {
      if (m_correction) delete m_correction;
      m_correction = correction;
      if (m_correction) m_correction->SetDirectory(m_directory);
    }

  private:
    TDirectory* m_directory; //<** TDirectory that holds m_helper and m_corrections.  */
    TTree* m_helper; //<** TTree that holds helper variables like bin boundaries. This class is the owner of the pointer and is responsible to delete it. */
    TTree* m_correction; //<** TTree that holds corrections. This class is the owner of the pointer and is responsible to delete it. */

    // 1: Initial version
    ClassDef(ECLShowerCorrectorLeakageCorrection, 1); /**< ClassDef */
  };
} // end namespace Belle2

