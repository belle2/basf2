/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VTX_ClusterProjection_H
#define VTX_ClusterProjection_H

namespace Belle2 {

  namespace VTX {

    /** Helper struct to collect information about the 1D projection of a Pixel cluster.
     * This class just collects some information necessary to calculate the
     * cluster position and errors: the minimum and maximum pixel row or
     * column as well as the corresponding sensor position and charge in
     * that row or column and the weighted (by charge) sum of positions for
     * center of gravity determination
     */
    class ClusterProjection {
    public:
      /** Constructor */
      ClusterProjection(): m_pos(0), m_min(-1), m_max(0), m_minCharge(0),
        m_maxCharge(0), m_minPos(0), m_maxPos(0), m_error(0), m_charge(0),
        m_size(0) {}

      /** Add Pixel information to the projection.
       * This will adjust the minimal and maximal coordinates and the
       * charge collected at these coordinates as well as increment the
       * weighted position to calculate the center of gravity
       * @param cell pixel coordinate
       * @param position pixel center position on the sensor
       * @param charge charge in that pixel
       */
      void add(unsigned int cell, float position, float charge);
      /** Finish calculation of center of gravity and set correct cluster size. Should only be used once */
      void finalize()
      {
        m_pos /= m_charge;
        m_size = m_max - m_min + 1;
      }

      /** Return the projected position of the cluster. Should only be used after a call to finalize */
      double getPos() const { return m_pos; }
      /** Return the error of the cluster */
      double getError() const { return m_error; }
      /** Return the projected size of the cluster. Should only be used after a call to finalize */
      unsigned int getSize() const { return m_size; }
      /** Return the minimum cell part of the cluster */
      unsigned int getMinCell() const { return m_min; }
      /** Return the maximum cell part of the cluster */
      unsigned int getMaxCell() const { return m_max; }
      /** Return the total charge of the cluster */
      double getCharge() const { return m_charge; }
      /** Return the charge in the minimum cell of the cluster */
      double getMinCharge() const { return m_minCharge; }
      /** Return the charge in the maximum cell of the cluster */
      double getMaxCharge() const { return m_maxCharge; }
      /** Return the center charge of the cluster, that is total charge minus minimum and maximum cell charge*/
      double getCenterCharge() const { return m_charge - m_minCharge - m_maxCharge; }
      /** Return the position of the minimum cell of the cluster */
      double getMinPos() const { return m_minPos; }
      /** Return the position of the maximum cell of the cluster */
      double getMaxPos() const { return m_maxPos; }

      /** Set the position of the cluster. Should only be used after a call to finalize */
      void setPos(double pos) { m_pos = pos; }
      /** Set the error of the cluster */
      void setError(double error) { m_error = error; }

    private:

      /** Center of gravity of the cluster */
      double m_pos;
      /** Minimum row or column of the cluster */
      unsigned int m_min;
      /** Maximum row or column of the cluster */
      unsigned int m_max;
      /** Charge collected in the minimum row/column */
      double m_minCharge;
      /** Charge collected in the maximum row/column */
      double m_maxCharge;
      /** Position of the minimum row/column */
      double m_minPos;
      /** Position of the maximum row/column */
      double m_maxPos;
      /** Position error of the cluster */
      double m_error;
      /** Charge of the full cluster */
      double m_charge;
      /** Projected size of the cluster */
      unsigned int m_size;
    };

    inline void ClusterProjection::add(unsigned int cell, float position, float charge)
    {
      //If the new cell is smaller than the last minimum cell than change that
      if (cell < m_min) {
        m_min = cell;
        m_minCharge = charge;
        m_minPos = position;
      } else if (cell == m_min) {
        //But if it is equal to the last known minimum cell sum up the charge
        m_minCharge += charge;
      }
      //Same as above for maximum cell.
      if (cell > m_max || m_max == 0) {
        m_max = cell;
        m_maxCharge = charge;
        m_maxPos = position;
      } else if (cell == m_max) {
        m_maxCharge += charge;
      }
      //Add weighted positions for center of gravity
      m_pos += charge * position;
      m_charge += charge;
    }

  } // VTX namespace

} // Belle2 namespace

#endif //VTX_ClusterProjection_H
