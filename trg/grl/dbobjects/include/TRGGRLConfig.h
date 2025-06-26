/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {



  /** The payload class for GRL parameters
   *
   */


  class TRGGRLConfig: public TObject {
  public:

    /** Default constructor */
    TRGGRLConfig(): m_ecltaunn_threshold{-1} {}

    /** Get MVA threshold of ecltaunn bit*/
    float get_ecltaunn_threshold() const
    {
      return m_ecltaunn_threshold;
    }
    /** Set MVA threshold of ecltaunn bit*/
    void set_ecltaunn_threshold(float i)
    {
      m_ecltaunn_threshold = i;
    }


  private:

    /** Number of TS */
    float m_ecltaunn_threshold;

    ClassDef(TRGGRLConfig, 2);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
