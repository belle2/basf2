/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHINFO_H
#define ARICHINFO_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {


  //! Datastore class to keep ARICH event infomation e.g. trigger type.
  class ARICHInfo : public RelationsObject {
  public:

    //! Empty constructor
    /*! Recommended for ROOT IO */
    ARICHInfo():
      m_trgtype(0),
      m_thscan_mode(0),
      m_vth_thscan(0.0)
    {
      /*! does nothing */
    }

    //! Useful Constructor
    /*!
      \param trgtype obtained from B2L data
    */
    explicit ARICHInfo(int trgtype)
    {
      m_trgtype = trgtype;
    }

    //! Set the trigger type
    void settrgtype(int trgtype) { m_trgtype = trgtype; }

    //! Set the threshold scan mode flag
    void setthscan_mode(bool mode) { m_thscan_mode = mode; }

    //! Set vth for thscan mode
    void setvth_thscan(double vth_thscan) { m_vth_thscan = vth_thscan; }

    //! Set number of tracks of this event
    void setntrack(double n_track) { m_n_track = n_track; }

    //! Set number of eligible ExtHits of this event
    void setnexthit(double n_exthit) { m_n_exthit = n_exthit; }

    //! Set number of ARICHHits of this event
    void setnhit(double n_hit) { m_n_hit = n_hit; }

    //! Get the trigger type
    int gettrgtype() const { return m_trgtype; }

    //! Get the threshold scan mode flag
    bool getthscan_mode() const { return m_thscan_mode; }

    //! Get vth for thscan
    double getvth_thscan() const { return m_vth_thscan; }

    //! Get number of tracks of this event
    int getntrack() const { return m_n_track; }

    //! Get number of eligible ExtHits of this event
    int getnexthit() const { return m_n_exthit; }

    //! Get number of ARICHHits of this event
    int getnhit() const { return m_n_hit; }

  private:

    int m_trgtype = 0;            /**< trigger type */
    bool m_thscan_mode = false;  /**< Flag of threshold scan run  */
    double m_vth_thscan = 0;            /**< vth value for thscan mode */
    int m_n_track = 0;            /**< number of tracks of this event */
    int m_n_exthit = 0;            /**< number of eligible ExtHits of this event */
    int m_n_hit = 0;            /**< number of ARICHHits of this event */



    ClassDef(ARICHInfo, 3); /**< the class title */

  };

} // end namespace Belle2

#endif  // ARICHINFO_H
