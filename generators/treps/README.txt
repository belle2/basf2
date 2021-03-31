========================================================================================

TREPS and TREPSST for basf2 (Belle II)   Reference Manual
            S.Uehara (KEK)            Nov.12, 2020
                              Rev.    Feb.1, 2021


--------------------------------------

TREPS (Two-photon to resonance process generator) -- For zero-tag configuration
TREPSST  -- For single tag configuration

--------------------------------------

1. Input parameters in Script

ParameterFile  string "name of parameter file for TREPS or TREPSST input" 
                           default="treps_par.dat"(TREPS) or "trepsst_par.dat"(TREPSST)
WListTableFile  string "file name for W-List table input. If UseDiscreteAndSortedW 
                      is false (default), the file is used."  default="wlist_table.dat"
DifferentialCrossSectionFile  string "file name for differential cross section table 
                      input. If UseDiscreteAndSortedW is true, the file is used"
                      default="pipidcs.dat"        (TREPS only)
UseDiscreteAndSortedW  bool  "if true, use WListTable for discrete and sorted W. 
                      if false, use DifferentialCrossSection" default=false
    * Only true is supported in TREPSST. As "false" is default, however, set "true", 
                                                       explicitly, for TREPSST.
MinimalQ21  double  "minimum value for Q^2 at the tag side for event generation in GeV^2" 
                     default=2.0  (TREPSST only)  
MaximalQ2  double  "maximum value for Q^2 for calculations and event generation in GeV^2.  
                    Negative means no cut" default=-1.0  (TREPS only)
MaximalQ22  double "maximum value for Q^2  at the untag side for calculations and event 
                    generation in GeV^2." default=1.0  (TREPSST only)
     * Maximal22 <= MinimalQ21 is the recommended condition to make the tag-side unambiguous.
MaximalRk  double "maximum value for fractional radiation energy for ISR for the beam 
                   at the tag side for event generation" default=0.25  (TREPSST only)
     * If you do not need ISR, set a value less than 0.0001.
MaximalAbsCosTheta  double  "maximum value for |cos(theta)| polar angle in e+e- c.m.s.
                   for any of the final-state particles from two-photon collisions" default=1.01
ApplyCosThetaCutCharged bool "whether to apply the cut |cos(theta)| for charged particles only"
                    default=true
MinimalTransverseMomentum  double "muinimum value for transverse momentum in e+e- c.m.s. for any 
                   of the final-state particles." default=0.0
ApplyTransverseMomentumCutCharged bool "whether to apply the transverse momentum cut for 
                   charged particles only", default=true
TagCharge  double  "electron charge at the tag side: -1.0:electron-tag or 1.0:positron-tag" 
                   default=-1.0  (TREPSST only)
BSTMode  int "Single-tag scattering calculation mode:  1:other general cases or 2:single 
                    P-meson case(as a pi0)" default=1   (TREPSST only)


2. Input parameters in ParameterFile

The 1st line:  Npart(int) PhysicsModel(int)  FFModel(int)
The 2nd to (1+Npart)th lines: PCode(int)  Mass(double)  Charge(double)  Ndec(int)  Width(double)
Later lines:  PCode(int)  Mass(double)  Charge(double)   

where,
Npart  Number of primary particles from two-photon collisions.  Npart>=2
PhysicsModel  Physics model applied for angular distribution or decay structures
FFModel       Form-factor parametrization model related to reaction rate of the untaged photon 
PCode     Particle code 
Mass      Particle mass in GeV/c2
Charge    Particle's electric charge
Ndec      Number of (secondary) decay particles
Width     Particle's decay width (Non-relativistic Breit-Wigner) in GeV

   * "Later lines" should be lined up of the order of the primary particles and must contain
     Sum(Ndec) lines in total.


3. Input parameters in WListTableFile

The 1st line:  INMode(int) 
The 2nd and later lines:     W(double)    Ne(int)
The last line:  StopW(double) 

INMode 0:Event generation, 1:lumi-func. W or S, 2:lumi-func. N
   * lumi-func. W or S is the ordinary luminosity function. lumi-func. N is
     the conversion factor dedicated to a narrow resonance.
     (INMode=2 is supported for TREPS only.)
W   gamma-gamma c.m. collision energy = invariant mass of the particles from 
    the gamma-gamma collision in GeV
Ne  Number of the generated events at this W
StopW   The stop code, which is StopW>9000 || StopW<0., in order to stop further 
        reading of this file.

* You must sepecify the total number of events to be generated in

  main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=(Nev))

  , where (Nev) should be in Sum of Ne, in principle, for INMode=0, for a clear
  consistency, although (Nev) have a priority.  If (Nev)<Sum(Ne), the
  event generation is stopped at (Nev) even on the way of WList. If (Nev)>Sum(Ne),
  the last W value is used to complete (Nev)-event generation.
    Even for case of INMode=1 or 2, specify (Nev)>=1, although dummy(empty) events 
  are generated.


4. Parameters in DifferentialCrossSectionFile
   (TREPS only)

Each line:  W(double)   DCS(double)

W     gamma-gamma c.m. collision energy in GeV
DCS   Differential cross section d(sig_ee)/dW in arbitrary unit
     (Only relative size is essential, but use of nb/GeV unit is recommended.)

* The cross section table is interpolated, linearly, when it is used.

----------------------------------------------

5. Physics Models
PhysicsModel =
0: General, isotropic(phase space)
201: 2+(0) --> PP
202: 2+(2) --> PP
251: pi+pi-  (fit to experimental angular distributions)
252: K+K-  (fit to experimental angular distributions)
253: ppbar fit to experimental angular distributions)
301: 0- -> VP -> 3P
302: 2+(0) -> VP -> 3P
303: 2+(2) -> VP -> 3P
304: 0- -> TP -> 3P
305: 2+(0) -> TP -> 3P
306: 2+(2) -> TP -> 3P
307: 2- -> SP -> 3P
308: 2- -> VP -> 3P
309: 2- -> TP -> 3P
331: 2+(0) -> VP -> PgP  (g is a photon)
332: 2+(2) -> VP -> PgP
333: 0+/- -> Vg -> PPg
334: 0+ -> Vg (E1) --> llg
335: 2+(0) -> Vg (E1) --> llg
336: 2+(2) -> Vg (E1) --> llg

* Int(PhysicsModel/100) corresponds to Npart, except 0.
* Symmetrization for Bose particles and any other resonance functions
  are considered, yet.  It is welcome to add Physics Models!
* Tools to extend to 4-body or to 5-body decays are prepared and
  partially implemented in the code.
* PhysicsMode<=202 only are supported in TREPSST.


6. Form-factor parametrization models 
FFMode = 
0: 1/(1+Q^2/W^2)  factor
1: 1/(1+Q^2/m_rho^2) factor
2: 1/(1+Q^2/m_Jpsi^2) factor
3: No factor (=1.0)   (3 is for TREPSST only)

-----------------------
Aknowledgements
 I thank K.Hayasaka-san and Y.Sato-san for their translation and 
installation to basf2 system.
-----------------------
Reference
TREPS: S. Uehara, KEK Report 96-11, arXiv:1310.0157 [hep-ph].

=====================================================

