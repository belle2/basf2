/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMDIGITIZERPAR_H
#define BKLMDIGITIZERPAR_H

#include <vector>
#include <string>

namespace Belle2 {

//! The Class for BKLM Digitizer Parameters
  /*! This class provides BKLM digitizer parameters for simulation.
      These parameters are obtained from Gearbox.
  */
  class BKLMDigitizerPar {

  public:

    //! Constructor
    BKLMDigitizerPar();

    //! Destructor
    virtual ~BKLMDigitizerPar();

    //! Static method to get a reference to the BKLMDigitizerPar instance.
    /*!
        \return A reference to an instance of this class.
    */

    static BKLMDigitizerPar* Instance();

    //! Clear all digitizer parameters
    void clear();

    //! Get digitizer parameters from Gearbox.
    void read();

    //! to get the maximum global time for a recorded sim hit
    const double hitTimeMax(void) const;

  private:

    //! variable for the maximum global time for a recorded sim hit
    double m_hitTimeMax;

    //! static pointer to the singleton instance of this class
    static BKLMDigitizerPar* m_BKLMDigitizerParDB;

  };

//-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif
