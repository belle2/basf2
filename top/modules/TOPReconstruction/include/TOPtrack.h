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

class TOPtrack {
public:
  // constructors
  TOPtrack();
  TOPtrack(double x, double y, double z, double Px, double Py, double Pz,
           double Tlen, int Q, int Lund = 0, int label = 0);
  // selectors
  double X() {return m_X;}
  double Y() {return m_Y;}
  double Z() {return m_Z;}
  double Px() {return m_Px;}
  double Py() {return m_Py;}
  double Pz() {return m_Pz;}
  double Tlen() {return m_Tlen;}
  double p();      // momentum
  double theta();  // polar angle
  double phi();    // azimuthal angle
  int Lund() {return m_LUND;}
  int Label() {return m_REF;}
  int Q() {return m_Q;}
  int Hyp();   // 1=e, 2=mu, 3=pi, 4=K, 5=p, 0=other
  int QbarID() {return m_QbarID;}
  bool atTop() {return m_atTop;}
  // modifiers
  int toTop(); // propagate track to TOP counter, return QbarID if hit else -1
  void smear(double sig_x, double sig_z, double sig_theta, double sig_phi);
  // print to std output
  void Dump();

private:
  double m_X;    // point
  double m_Y;    // point
  double m_Z;    // point
  double m_Px;   // momentum
  double m_Py;   // momentum
  double m_Pz;   // momentum
  double m_Tlen; // track length from IP to point
  int m_Q;       // charge
  int m_LUND;    // LUND code (optional)
  int m_REF;     // reference label (optional)
  bool m_atTop;  // true, if toTop() called
  int m_QbarID;  // Qbar ID or -1
};

#endif

