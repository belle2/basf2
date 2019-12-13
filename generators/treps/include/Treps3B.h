// -*- C++ -*-
//
// Package:     <package>
// Module:      Treps3B
//
// Description: <one line class summary>
//  Class name is TrepsB
// Usage:
//    <usage>
//
// Author:      Sadaharu Uehara
// Created:     Jul.17 1997
// $Id$
//
// Revision history
//
// Modified: To use RandFlat instead of bare HepRandom
//         9-MAY-2000 S.Uehara
// $Log$



#if !defined(PACKAGE_TREPS3B_H_INCLUDED)
#define PACKAGE_TREPS3B_H_INCLUDED

#include<TVector3.h>
#include<TLorentzVector.h>
#include<TH1F.h>
#include<TTree.h>
#include<TString.h>
#include <generators/treps/Sutool.h>
#include <generators/treps/Particle_array.h>

namespace Belle2 {


  class TrepsB {

  public:
    // constants, enums and typedefs
    double w;      // invariant mass of two-photon system
    char parameterFile[132]; // filename for parameter input
    char wlistFile[132]; // filename for W List input
    char diffcrosssectionFile[132]; // filename for differential cross section

    TString filnam_hist; // filename for HBOOK histogram output
    int ntot, nsave ; // number of events generated and saved
    Sutool sutool; //calculation tool kit by S.U

    // Constructors and destructor
    TrepsB(void);
    ~TrepsB(void) {};

    // member functions
    void initp(void);
    void create_hist(void);
    int event_gen(int);
    void setW(double);
    void updateW(void);
    double twlumi(void);
    double twlumi_n(void);
    void tpkin3(Part_gen*,
                int, int, int, double&, double&, double&,
                double&, double&, double&, double&) ;
    void tpkin4(Part_gen*,
                int, int, int, int, double&, double&, double&,
                double&, double&, double&, double&, double&,
                double&, double&, double&, double&) ;
    void tpkin5(Part_gen*,
                int, int, int, int, int, double&, double&, double&,
                double&, double&, double&, TVector3&,
                TVector3&, TVector3&, double&) ;
    virtual int tpuser(TLorentzVector, TLorentzVector,
                       Part_gen*, int);
    //double rnd8(void){
    //    extern BasfranEngine* BASF_Random_Engine;
    //    double fnum = RandFlat::shoot( BASF_Random_Engine );
    //    return fnum;
    //  }


    // const member functions
    void terminate(void) const ;
    virtual double tpform(double, double) const  ;
    virtual double tpangd(double, double)  ;
    virtual void trkpsh(int, TLorentzVector, TLorentzVector,
                        Part_gen*, int) const ;
    virtual void print_event(void) const ;

    //201903
    void wtable();
    double wtable(int);
    //201903E



  private:

    // private member functions
    double tpgetd(int, double, double, double,  double);
    double tpgetz(int);
    double tpgetq(double, double, double);
    void tpgetm(Part_cont*, int);

    // private const member functions
    double simpsn1(double, double, int) const ;
    double simpsn2(double, double, int) const ;
    double tpxint(double, double, double) const ;
    double tpf(double, double) const ;
    double tpdfnc(double) const ;
    double tplogf(double) const ;
    double simpsny(double, double, double, int) const ;
    double simpsnz(double, double, double, int) const ;
    double tpdfy(double, double) const;
    double tpdfz(double, double) const;

    // data members
    double ebeam;  // beam energy in cm system = sqrt(s)/2
    //const double q2zero = 1.0e-6; // minimum Q^2 of finite
    constexpr const static  double q2zero = 1.0e-6;
    double q2max; // q^2 max
    double cost_cut, pt_cut ;  // cut for save
    int cost_flag, pt_flag ; // flag watching neutral particles
    //    const double pi = 3.14159265358979;     // pi
    constexpr const static double pi = 3.14159265358979;     // pi
    //    const double twopi = 2.0*pi;
    constexpr const static double twopi = 2.0 * 3.14159265358979;
    TVector3 pfeb, pfpb, tswsb ;
    double qzmin, qptmin;
    //int ndecay ;
    // Part_cont parti[10], parts[20] ;  // particle properties of final-state
    Part_cont* parti, *parts ;
    double ephi, etheta ;
    double s ;
    int imode ;
    double dmin, dmax ;
    double ffmax ;
    double rs;
    double vmax;
    TLorentzVector* pppp ;

    //201903
  public:
    float wf;
    int inmode;
    int wtcount;
    int  wthead[5000];
    double  wtcond[5000];

    // Table of differential cross section of W.
    std::map<double, double> diffCrossSectionOfW;

    // They are used for simulate W distribution with importance sampling. NOT CORRECT cross section !!
    double totalCrossSectionForMC;
    std::map<double, double> WOfCrossSectionForMC;

    //201903E
    int ndecay;  // Num of particles at the first stage
    int pmodel; // Physics model
    int fmodel; // Form factor model

  public:
    TLorentzVector peb, ppb ; // initial-state e+ e- 4-momenta in lab.
    int ievent;
    TH1F* treh1, * treh2, * treh3, * treh4, * treh5, * treh6 ;




  protected:
    int npart;
    Part_gen* partgen;  // final-state particle data in lab.
    TLorentzVector pe, pp ;  // final-state e+ e- 4-momenta in lab.
    //   const double me = 0.000510999;     // electron mass
    constexpr const static double me = 0.000510999;     // electron mass
    TH1F* zdis, *zpdis, *zsdis;
  };


#endif /* PACKAGE_TREPS3B_H_INCLUDED */

} // namespace Belle2
