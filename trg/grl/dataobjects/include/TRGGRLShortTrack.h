/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGRLShortTrack.h
// Section  : TRG GRL
// Owner    : Yun-Tsung Lai
// Email    : ytlai@post.kek.jp
//-----------------------------------------------------------
// Description : storeArray for TRG GRL CDC-KLM matching.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGGRLShortTrack_H
#define TRGGRLShortTrack_H

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /// a class for neutral ECL cluster in TRGGRL
  class TRGGRLShortTrack : public RelationsObject {
  public:

    /// Constructor
    TRGGRLShortTrack(): m_TS_ID( {0}) {}
    /// Destructor
    ~TRGGRLShortTrack() {}

    /**Get TS ID of SL i, from 0 to 63*/
    double get_TS_ID(int i) const {return m_TS_ID[i];}

    /**Get TS phi of SL i with unit = 5.625*/
    double get_TS_phi(int i) const {return m_TS_ID[i] * 5.625;}

    /** set TS ID of SL i*/
    void set_TS_ID(int i, int id) { m_TS_ID[i] = id; }


    /** initialization*/
    void init()
    {
      m_TS_ID.clear();
      for (int i = 0; i < 5; i++) {
        m_TS_ID.push_back(-1);
      }
    }

  private:

    /**TSF ID used for the short track*/
    std::vector<int> m_TS_ID = std::vector<int>(0, 0);

    /**! The Class title*/
    ClassDef(TRGGRLShortTrack, 1); /*< the class title */

  };

} // end namespace Belle2

#endif
