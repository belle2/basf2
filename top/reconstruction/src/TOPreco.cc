//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, April-2009, Sept-2011
//-----------------------------------------------------------------------------
//
// TOPreco.cc
// C++ interface to F77 functions: reconstruction
//-----------------------------------------------------------------------------
//*****************************************************************************

#include <iostream>
#include <iomanip>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPf77fun.h>
#include <top/geometry/TOPGeometryPar.h>

#include <framework/gearbox/Const.h>

namespace Belle2 {
  namespace TOP {

    TOPreco::TOPreco(int Num, double Masses[], double BkgPerQbar, double ScaleN0):
      m_hypID(0), m_beta(0.0)
    {
      data_clear_();
      rtra_clear_();
      std::vector<float> masses;
      for (int i = 0; i < Num; i++) {
        masses.push_back((float) Masses[i]);
      }
      rtra_set_hypo_(&Num, masses.data());
      float b = (float) BkgPerQbar; float s = (float) ScaleN0;
      set_top_par_(&b, &s);
      setPDFoption(c_Optimal); // default option
      setTmax(0); // use default (TDC range)
      setBeta(0); // use default: beta from momentum and mass
    }

    void TOPreco::setHypID(int NumHyp, int HypID[])
    {
      rtra_set_hypid_(&NumHyp, HypID);
    }

    void TOPreco::setMass(double Mass)
    {
      int Num = 1;
      float mass = (float) Mass;
      rtra_set_hypo_(&Num, &mass);
      int HypID = 0;
      rtra_set_hypid_(&Num, &HypID);
    }

    void TOPreco::clearData()
    {
      data_clear_();
      rtra_clear_();
    }

    int TOPreco::addData(int barID, int pixelID, int TDC, double time)
    {
      int status;
      barID--; // 0-based ID used in fortran
      pixelID--;   // 0-based ID used in fortran
      float t = (float) time;
      data_put_(&barID, &pixelID, &TDC, &t, &status);
      return status;
    }


    int TOPreco::addData(int barID, int pixelID, int TDC)
    {
      int status;
      barID--; // 0-based ID used in fortran
      pixelID--;   // 0-based ID used in fortran
      float t = TOPGeometryPar::Instance()->getTime(TDC);
      data_put_(&barID, &pixelID, &TDC, &t, &status);
      return status;
    }


    void TOPreco::reconstruct(double X, double Y, double Z, double Tlen,
                              double Px, double Py, double Pz, int Q,
                              int HYP, int barID)
    {
      float x = (float) X;
      float y = (float) Y;
      float z = (float) Z;
      float t = float(Tlen / Const::speedOfLight);
      float px = (float) Px;
      float py = (float) Py;
      float pz = (float) Pz;
      int REF = 0;
      barID--; // 0-based ID used in fortran
      rtra_clear_();
      rtra_put_(&x, &y, &z, &t, &px, &py, &pz, &Q, &HYP, &REF, &barID);
      top_reco_();
    }

    void TOPreco::reconstruct(TOPtrack& trk, int pdg)
    {
      m_hypID = abs(trk.getPDGcode());
      if (pdg == 0) pdg = m_hypID;
      int barID = trk.getBarID();
      reconstruct(trk.getX(), trk.getY(), trk.getZ(), trk.getTrackLength(),
                  trk.getPx(), trk.getPy(), trk.getPz(), trk.getCharge(),
                  pdg, barID);
    }

    int TOPreco::getFlag()
    {
      int K = 1;
      return rtra_get_flag_(&K);
    }

    double TOPreco::getExpectedPhotons(int i)
    {
      int K = 1;
      i++;
      return rtra_get_sfot_(&K, &i);
    }

    double TOPreco::getExpectedBG()
    {
      int K = 1;
      return rtra_get_bfot_(&K);
    }

    int TOPreco::getNumOfPhotons()
    {
      int K = 1;
      return rtra_get_nfot_(&K);
    }

    double TOPreco::getLogL(int i)
    {
      int K = 1;
      i++;
      return rtra_get_plkh_(&K, &i);
    }

    void TOPreco::getLogL(int Size, double LogL[], double ExpNphot[], int& Nphot)
    {
      int K = 1;
      std::vector<float> logl(Size), sfot(Size);
      int Flag, MTRA, REF;
      rtra_get_(&K, logl.data(), sfot.data(), &Size, &Nphot, &Flag, &MTRA, &REF);
      for (int i = 0; i < Size; i++) {
        LogL[i] = logl[i];
        ExpNphot[i] = sfot[i];
      }
    }

    void TOPreco::getTrackHit(int LocGlob, double R[3], double Dir[3], double& Len,
                              double& Tlen, double& Mom, int& barID)
    {
      int K = 1;
      float r[3], dir[3], len, tof, p;
      rtra_gethit_(&K, &LocGlob, r, dir, &len, &tof, &p, &barID);
      barID++;
      for (int i = 0; i < 3; i++) {
        R[i] = r[i];
        Dir[i] = dir[i];
      }
      Len = len; Tlen = tof * Const::speedOfLight; Mom = p;
    }

    void TOPreco::dumpLogL(int Size)
    {
      std::vector<double> logl(Size), sfot(Size);
      std::vector<int> hypid(Size);
      int Nphot;
      getLogL(Size, logl.data(), sfot.data(), Nphot);
      rtra_get_hypid_(&Size, hypid.data());

      int i_max = 0;
      double logl_max = logl[0];
      for (int i = 1; i < Size; i++) {
        if (logl[i] > logl_max) {logl_max = logl[i]; i_max = i;}
      }

      using namespace std;
      cout << "TOPreco::dumpLogL: Flag=" << getFlag();
      cout << "  Detected Photons=" << Nphot << endl;
      cout << " i         HypID   LogL   ExpPhot" << endl;
      cout << showpoint << fixed << right;
      for (int i = 0; i < Size; i++) {
        cout << setw(2) << i;
        cout << setw(12) << hypid[i];
        cout << setw(10) << setprecision(2) << logl[i];
        cout << setw(8) << setprecision(2) << sfot[i];
        if (i == i_max) cout << " <";
        if (hypid[i] == m_hypID) cout << " <-- truth";
        cout << endl;
      }
    }

    void TOPreco::dumpTrackHit(int LocGlob)
    {
      double r[3], dir[3], len, Tlen, p;
      int barID;
      getTrackHit(LocGlob, r, dir, len, Tlen, p, barID);

      using namespace std;
      cout << showpoint << fixed << right;
      cout << "TOPreco::dumpTrackHit: barID=" << barID;
      cout << "  Len=" << setprecision(2) << len;
      cout << "cm  Tlen=" << setprecision(1) << Tlen;
      cout << "cm  p=" << setprecision(2) << p << "GeV/c" << endl;
      cout << "position [cm]: ";
      for (int i = 0; i < 3; i++) {
        cout << setw(10) << setprecision(2) << r[i];
      }
      if (LocGlob == Local) {cout << " (local)" << endl;}
      else {cout << " (global)" << endl;}

      cout << "direction:     ";
      for (int i = 0; i < 3; i++) {
        cout << setw(10) << setprecision(4) << dir[i];
      }
      if (LocGlob == Local) {cout << " (local)" << endl;}
      else {cout << " (global)" << endl;}
    }

    int TOPreco::getPullSize()
    {
      int n;
      getnum_pulls_(&n);
      return n;
    }

    void TOPreco::getPull(int K, int& ich, float& t, float& t0, float& wid,
                          float& fic, float& wt)
    {
      K++;
      get_pulls_(&K, &t, &t0, &wid, &fic, &wt, &ich);
      ich++; // convert to 1-based
    }

    double TOPreco::getPDF(int pixelID, double T, double Mass)
    {
      float t = (float) T; float mass = (float) Mass;
      return get_pdf_(&pixelID, &t, &mass);
    }

  } // end top namespace
} // end Belle2 namespace

