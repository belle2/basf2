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

#include <math.h>

#ifndef _TOPtrack_h
#define _TOPtrack_h

namespace Belle2 {
  namespace TOP {

    /**
     * Class to hold reconstructed track, interface to fortran.
     * Input to TOPreco.
     */

    class TOPtrack {
    public:
      /**
       * Default constructor
       */
      TOPtrack();

      /**
       * Constructor with track parameters
       * @param x track spatial position x
       * @param y track spatial position y
       * @param z track spatial position z
       * @param Px track momentum component x
       * @param Py track momentum component y
       * @param Pz track momentum component z
       * @param Tlen track length from IP
       * @param Q track charge
       * @param pdg PDG code (optional)
       */
      TOPtrack(double x, double y, double z, double Px, double Py, double Pz,
               double Tlen, int Q, int pdg = 0);

      /**
       * Return position component
       * @return  spatial position x
       */
      double getX() {return m_x;}

      /**
       * Return position component
       * @return  spatial position y
       */
      double getY() {return m_y;}

      /**
       * Return position component
       * @return  spatial position z
       */
      double getZ() {return m_z;}

      /**
       * Return momentum component
       * @return  momentum component x
       */
      double getPx() {return m_px;}

      /**
       * Return momentum component
       * @return  momentum component y
       */
      double getPy() {return m_py;}

      /**
       * Return momentum component
       * @return  momentum component z
       */
      double getPz() {return m_pz;}

      /**
       * Return track length from IP to current position
       * @return track length from IP
       */
      double getTrackLength() {return m_trackLength;}

      /**
       * Return time-of-flight from IP to current position for given charged stable
       * @param PDGcode PDG code
       * @return time-of-flight
       */
      double getTOF(int PDGcode);

      /**
       * Set track length from time-of-flight and particle mass
       * @param tof time-of-flight
       * @param mass particle mass
       */
      void setTrackLength(double tof, double mass);

      /**
       * Return momentum magnitude
       * @return momentum
       */
      double getP() {return sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);}

      /**
       * Return momentum polar angle
       * @return momentum polar angle
       */
      double getTheta() {return acos(m_pz / getP());}

      /**
       * Return momentum azimuthal angle
       * @return momentum azimuthal angle
       */
      double getPhi() {return atan2(m_py, m_px);}

      /**
       * Return PDG code
       * @return PDG code
       */
      int getPDGcode() {return m_pdg;}

      /**
       * Return first label
       * @return label
       */
      int getLabel() {return m_label[0];}

      /**
       * Return label i
       * @param i label index (i=0...9)
       * @return label
       */
      int getLabel(unsigned int i) {if (i < 10) {return m_label[i];} else {return 0;}}

      /**
       * Set label i
       * @param i label index (i=0...9)
       * @param label label to store
       */
      void setLabel(unsigned int i, int label) {if (i < 10) m_label[i] = label;}

      /**
       * Return charge
       * @return charge
       */
      int getCharge() {return m_charge;}

      /**
       * Return internal particle code
       * @return internal particle code: 1=e, 2=mu, 3=pi, 4=K, 5=p, 0=unknown
       */
      int getHypID();

      /**
       * Return bar ID
       * @return bar ID, if track hits the bar, otherwise 0
       */
      int getBarID() {return m_barID;}

      /**
       * Check if toTop() was already called
       * @return true or false
       */
      bool atTop() {return m_atTop;}

      /**
       * Propagate track to TOP counter (assuming uniform B field along z)
       * @return bar ID, if track hits the bar, otherwise 0
       */
      int toTop();

      /**
       * Smear track
       * @param sig_x sigma in x
       * @param sig_z sigma in z
       * @param sig_theta sigma in theta
       * @param sig_phi sigma in phi
       */
      void smear(double sig_x, double sig_z, double sig_theta, double sig_phi);

      /**
       * print track parameters to std output
       */
      void dump();

    private:
      double m_x;      /**< point */
      double m_y;      /**< point */
      double m_z;      /**< point */
      double m_px;     /**< momentum */
      double m_py;     /**< momentum */
      double m_pz;     /**< momentum */
      double m_trackLength;   /**< track length from IP to point */
      int m_charge;    /**< charge */
      int m_pdg;       /**< PDG code (optional) */
      int m_label[10]; /**< labels (optional) */
      bool m_atTop;    /**< true, if toTop() called */
      int m_barID;     /**< bar ID or -1 */
    };

  } // end top namespace
} // end Belle2 namespace

#endif

