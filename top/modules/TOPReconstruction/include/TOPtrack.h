//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, Jan-2010, March-2010, Sept-2011
//-----------------------------------------------------------------------------
//
// TOPtrack.h
// C++ interface to F77 functions: reconstructed track
//-----------------------------------------------------------------------------
//*****************************************************************************

#ifndef _TOPtrack_h
#define _TOPtrack_h

namespace Belle2 {
  namespace TOP {

    /*! Class to hold reconstructed track, interface to fortran
     *  Input to TOPreco
     */

    class TOPtrack {
    public:
      /*! Default constructor
       */
      TOPtrack();

      /*! Constructor with track parameters
       * @param x track spatial position x
       * @param y track spatial position y
       * @param z track spatial position z
       * @param Px track momentum component x
       * @param Py track momentum component y
       * @param Pz track momentum component z
       * @param Tlen track length from IP
       * @param Q track charge
       * @param Lund LUND code (optional)
       * @param label label (optional)
       */
      TOPtrack(double x, double y, double z, double Px, double Py, double Pz,
               double Tlen, int Q, int Lund = 0, int label = 0);

      /*! get x
       * @return  spatial position x
       */
      double X() {return m_X;}

      /*! get y
       * @return  spatial position y
       */
      double Y() {return m_Y;}

      /*! get z
       * @return  spatial position z
       */
      double Z() {return m_Z;}

      /*! get momentum x
       * @return  momentum component x
       */
      double Px() {return m_Px;}

      /*! get momentum y
       * @return  momentum component y
       */
      double Py() {return m_Py;}

      /*! get momentum z
       * @return  momentum component z
       */
      double Pz() {return m_Pz;}

      /*! get track length from IP to current position
       * @return track length from IP
       */
      double Tlen() {return m_Tlen;}

      /*! get momentum magnitude
       * @return momentum
       */
      double p();

      /*! get momentum polar angle
       * @return momentum polar angle
       */
      double theta();

      /*! get momentum azimuthal angle
       * @return momentum azimuthal angle
       */
      double phi();

      /*! get LUND code
       * @return LUND code
       */
      int Lund() {return m_LUND;}

      /*! get label
       * @return label
       */
      int Label() {return m_REF;}

      /*! get charge
       * @return charge
       */
      int Q() {return m_Q;}

      /*! get internal particle code
       * @return internal particle code: 1=e, 2=mu, 3=pi, 4=K, 5=p, 0=other
       */
      int Hyp();

      /*! get bar ID
       * @return bar ID if hit, else -1
       */
      int QbarID() {return m_QbarID;}

      /*! check if track is at TOP
       * @return true or false
       */
      bool atTop() {return m_atTop;}

      /*! propagate track to TOP counter
       * @return bar ID if hit, else -1
       */
      int toTop();

      /*! smear track
       * @param sig_x sigma in x
       * @param sig_z sigma in z
       * @param sig_theta sigma in theta
       * @param sig_phi sigma in phi
       */
      void smear(double sig_x, double sig_z, double sig_theta, double sig_phi);

      /*! print track parameters to std output
       */
      void Dump();

    private:
      double m_X;    /**< point */
      double m_Y;    /**< point */
      double m_Z;    /**< point */
      double m_Px;   /**< momentum */
      double m_Py;   /**< momentum */
      double m_Pz;   /**< momentum */
      double m_Tlen; /**< track length from IP to point */
      int m_Q;       /**< charge */
      int m_LUND;    /**< LUND code (optional) */
      int m_REF;     /**< label (optional) */
      bool m_atTop;  /**< true, if toTop() called */
      int m_QbarID;  /**< bar ID or -1 */
    };

  } // end top namespace
} // end Belle2 namespace

#endif

