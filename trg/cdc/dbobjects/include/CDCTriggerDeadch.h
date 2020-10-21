/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <unordered_map>
#include <vector>
#include <cdc/geometry/CDCGeometryPar.h>

namespace Belle2 {


  /** The payload class for delay of GDL input bit
   *
   *  The payload class stores the number of input bit and their delay
   *  value of a missing delay is "-1"
   */


  class CDCTriggerDeadch: public TObject {
  public:

    /** Default constructor */
    CDCTriggerDeadch()
    {
      for (unsigned int i = 0; i < nSuperLayers; i++) {
        for (unsigned int j = 0; j < MAX_N_LAYERS; j++) {
          for (unsigned int k = 0; k < MAX_N_SCELLS; k++) {
            m_deadch[i][j][k] = true;
          }
        }
      }
    }
    /** copy constructor */
    CDCTriggerDeadch(const CDCTriggerDeadch& b): TObject(b)
    {
      for (unsigned int i = 0; i < nSuperLayers; i++) {
        for (unsigned int j = 0; j < MAX_N_LAYERS; j++) {
          for (unsigned int k = 0; k < MAX_N_SCELLS; k++) {
            m_deadch[i][j][k] = b.m_deadch[i][j][k];
          }
        }
      }
    }
    /** assignment operator */
    CDCTriggerDeadch& operator=(const CDCTriggerDeadch& b)
    {
      for (unsigned int i = 0; i < nSuperLayers; i++) {
        for (unsigned int j = 0; j < MAX_N_LAYERS; j++) {
          for (unsigned int k = 0; k < MAX_N_SCELLS; k++) {
            m_deadch[i][j][k] = b.m_deadch[i][j][k];
          }
        }
      }
      return *this;
    }

    /** Get the number of TS */
    bool getdeadch(int sl, int layer, int ch) const
    {
      return m_deadch[sl][layer][ch];
    }
    /** Set the number of TS */
    void setdeadch(int sl, int layer, int ch, bool flag)
    {
      m_deadch[sl][layer][ch] = flag;
    }


  private:

    //** number of layers in Super layer**/
    const static int MAX_N_LAYERS = 8;

    /** Dead channel mapping in each superlayer, layer, ch*/
    bool m_deadch[nSuperLayers][MAX_N_LAYERS][MAX_N_SCELLS];

    ClassDef(CDCTriggerDeadch, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
