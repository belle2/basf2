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


#ifndef PACKAGE_TREPS3B_H_INCLUDED
#define PACKAGE_TREPS3B_H_INCLUDED

#include<TVector3.h>
#include<TLorentzVector.h>
#include<TH1F.h>
#include<TTree.h>
#include<TString.h>
#include <generators/treps/Sutool.h>
#include <generators/treps/Particle_array.h>

#include <string>

namespace Belle2 {


  class TrepsB {

  public:
    // Constructors and destructor
    TrepsB(void);
    ~TrepsB() {};

    // constants, enums and typedefs
    double w;      // invariant mass of two-photon system
    TString filnam_hist; // filename for HBOOK histogram output
    int ntot, nsave ; // number of events generated and saved
    Sutool sutool; //calculation tool kit by S.U


    // member functions
    void setParameterFile(std::string file)
    {
      parameterFile = file;
    }
    void setWlistFile(std::string file)
    {
      wlistFile = file;
    }
    void setDiffcrosssectionFile(std::string file)
    {
      diffcrosssectionFile = file;
    }

    void initp(void); // initialize the generator. read parameterFile and load the parameters
    void wtable(); // read diffcrosssectionFile and load W-DifferentialCrossSection table.
    double wtable(int); // read wlistFile and load W-NumberOfEvents table.
    void create_hist(void); // create histograms. it is just for debug purpose.
    int event_gen(int); // generate one event with given parameter and W
    void setW(double); // set given double to w (member variable) and call updateW
    void updateW(void); // update W and some related parameters
    double twlumi(void); // calculate and return two-photon luminosity function
    double twlumi_n(void); // calculate and return two-photon luminosity function for very narrow resonance
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

    // const member functions
    void terminate(void) const ;
    virtual double tpform(double, double) const  ; // returns form factor effect, actually always returns 1
    virtual double tpangd(double, double)  ; // always returns 1
    virtual void trkpsh(int, TLorentzVector, TLorentzVector,
                        Part_gen*, int) const ; // do nothing..
    virtual void print_event(void) const ; // print event information at B2DEBUG(10)


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
    std::string parameterFile; // filename for parameter input
    std::string wlistFile; // filename for W List input
    std::string diffcrosssectionFile; // filename for differential cross section

    double ebeam;  // beam energy in cm system = sqrt(s)/2
    constexpr const static  double q2zero = 1.0e-6;
    double q2max; // q^2 max
    double cost_cut, pt_cut ;  // cut for save
    int cost_flag, pt_flag ; // flag watching neutral particles
    constexpr const static double pi = 3.14159265358979;     // pi
    constexpr const static double twopi = 2.0 * 3.14159265358979; // 2pi
    TVector3 pfeb, pfpb; // Momentum of e(lectron) and p(ositron) at lab frame. Scaled to 1/ebeam.
    TVector3 tswsb ; // Normalized boost factor
    double qzmin, qptmin; // Minimum of qz and qpt
    Part_cont* parti, *parts ; // Particles to be generated. parti is primary, parts is secondary particles.
    double ephi, etheta ; // phi and theta of e+e- system at lab frame
    double s ; // (2*ebeam)^2
    int imode ; //
    double dmin, dmax ;
    double ffmax ;
    double rs;
    double vmax;
    TLorentzVector* pppp ; // Particles to be generated in TLorentzVector

  public:
    float wf; // w to set from outside of the class

    // wlisttableFile mode
    int inmode; // mode to control setting of wtable(int)
    int wtcount; // event number counter
    double  wtcond[5000]; // list of W
    int  wthead[5000]; // list of sum-of-NumberOfEvents up to current W

    // diffcrosssectionFile mode
    // Table of differential cross section of W.
    std::map<double, double> diffCrossSectionOfW;

    // They are used for simulate W distribution with importance sampling. NOT CORRECT cross section !!
    double totalCrossSectionForMC; // total cross section
    std::map<double, double> WOfCrossSectionForMC; // W and sum-of-crossSection up to current W

    int ndecay;  // Num of particles at the first stage
    int pmodel; // Physics model
    int fmodel; // Form factor model

  public:
    TLorentzVector peb, ppb ; // initial-state e+ e- 4-momenta in lab.
    int ievent; // event number
    TH1F* treh1, * treh2, * treh3, * treh4, * treh5, * treh6 ; // Histograms for debug


    // protected:
    int npart; // Number of particles to be generated
    Part_gen* partgen;  // final-state particle data in lab.
    TLorentzVector pe, pp ;  // final-state e+ e- 4-momenta in lab.
    constexpr const static double me = 0.000510999;     // electron mass
    TH1F* zdis, *zpdis, *zsdis;

  };


} // namespace Belle2

#endif /* PACKAGE_TREPS3B_H_INCLUDED */
