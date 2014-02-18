/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXD_PIXEL_H
#define PXD_PIXEL_H

#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDigit.h>

namespace Belle2 {

  namespace PXD {
    /**
     * Class to represent one pixel, used in clustering for fast access
     *
     * This class is a wrapper around PXDDigit or PXDRawHit to provide ordered
     * access. Ordering is row-wise: first all digits/rawhits belonging to the
     * first row, ordered by column in ascending order.
     *
     * The Pixel object contains a copy of the essential values of the PXDDigit
     * or PXDRawHit to speed up lookup during sorting or clustering. PXDDigit
     * or RawHit is not used directly as
     * a) we need the index of the digit after clustering and the digit
     * does not keep its own index and
     * b) Inheriting from TObject makes the PXDDigit/PXDRawHit more than three
     * times larger than this small class
     */
    class Pixel {
    public:
      /** Construct using only an index, used for testing. */
      Pixel(unsigned int index = 0): m_index(index), m_u(0), m_v(0),
        m_charge(0) {}
      /** Construct from a given PXDDigit and its store index */
      Pixel(const PXDDigit* digit, unsigned int index): m_index(index),
        m_u(digit->getUCellID()), m_v(digit->getVCellID()),
        m_charge(digit->getCharge()) {}
      /** Construct from a given PXDRawHit and its store index */
      Pixel(const PXDRawHit* rawhit, unsigned int index): m_index(index),
        m_u(rawhit->getColumn()), m_v(rawhit->getRow()),
        m_charge(rawhit->getCharge()) {}

      /** Comparison operator, sorting by row,column in ascending order */
      bool operator<(const Pixel& b) const { return m_v < b.m_v || (m_v == b.m_v && m_u < b.m_u); }
      /** Comparison operator, sorting by row,column in ascending order */
      bool operator<=(const Pixel& b) const { return  m_v < b.m_v || (m_v == b.m_v && m_u <= b.m_u); }
      /** Comparison operator, sorting by row,column in ascending order */
      bool operator>(const Pixel& b) const { return b < *this; }
      /** Comparison operator, sorting by row,column in ascending order */
      bool operator>=(const Pixel& b) const { return  b <= *this; }
      /** Equality operator */
      bool operator==(const Pixel& b) const { return m_v == b.m_v && m_u == b.m_u; }

      /** Return the CellID in u */
      unsigned short getU() const { return m_u; }
      /** Return the CellID in v */
      unsigned short getV() const { return m_v; }
      /** Return the Charge of the Pixel */
      float getCharge() const { return m_charge; }
      /** Return the Index of the digit */
      unsigned int getIndex() const { return m_index; }

    private:
      /** Index of the corresponding PXDDigit in the StoreArray */
      unsigned int m_index;
      /** Cell ID in u */
      unsigned short m_u;
      /** Cell ID in v */
      unsigned short m_v;
      /** Charge of the pixel */
      float m_charge;
    };

  } // PXD namespace
} // Belle2 namespace

#endif //PXD_PIXEL_H
