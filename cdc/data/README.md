# Relation betw. wireID and frontend electronicsID

Format: super layerID (0-8), layerID in the super-layer (0-), wireID(0-), boardID (0-299), channelID (0-47).

* ch_map.dat: coded by Yamada (DAQ group).


# xt-relation in new format

* change wrt old format file 
 - add incident angle bin info. at the top of a file in old format.
 - add xt parameterization mode and no. of params. after that. 
   mode=0: normal    polynomial;
       =1: Chebyshev polynomial.
 - shift alpha points: -80 - 90deg --> -85 - 85deg.
 - sort in order of alpha
 (- Unit: cm and nsec).
 (- unsigned driftL).
* xt_v3.0.1_chebyshev.dat.gz : went back to xt_v2.1.dat; modified strange two
                               bins (lyr=9,lr=0,thet=60,alpha=60 and
                               lyr=11,lr=0,theta=60,alpha=50); recreated simple
                               poly. ver. of xt.dat; then converted to
                               chebyshev poly. version.
* xt_v3_chebyshev.dat.gz     : created from xt_v3.
* xt_v3.dat                  : created from xt_v2.1 <- obsolete.
* xt_noB_v1_chebyshev.dat.gz : created from xt_noB_v1.
* xt_noB_v1.dat              : created from xt_noB_v0.3 <- obsolete.

# xt-relation in old format

Format: lyr#  theta  alpha phi left/right 0th 1st 2nd 3rd 4th 5th cut-off slope
lyr#: layerID (0-55);
theta: polar angle of the incident track (18-149deg.);
alpha: incident angle of the track in r-phi plane (0-180 deg.);
phi: azimuthal angle of the cell (not used now; maybe used in future);
left/right: left or right flag;
0th - 5th: coefficients for 5th-order polynomial;
cut-off: maximum drift time to use 5th order polynomial;
slope: if drift time is greater than cut-off, xt is approximated by linear
function. this is the slope for that.

Unit: cm and nsec.
* xt_v2.2.dat       : =xt_v2.1     but time-unit=nsec and unsigned driftL.
* xt_noB_v0.4.dat   : =xt_noB_v0.3 but ibid. 
* xt_simple_v0.1.dat: =xt_simple   but ibid. 
Unit: cm and microsec.
* xt_v2.1.dat    : obsolete. layer#55 thickness reduced by ~1cm.
* xt_v2.dat      : obsolete. xt = xt(layer,l/r,alpha,theta) with a bug fixed.
* xt_v1.dat      : obsolete. xt = xt(layer,l/r,alpha.theta)
* xt_v0.dat      : obsolete. 1'st version of realistic xt; only layer and L/R dependence is taken into account.
* xt_simple.dat  : obsolete. ideal linear xt. v=40 mu/ns.
* xt_noB_v0.3.dat: obsolete. 
                              xt for B-field = 0 ([cdc-soft:00383] ).
                              Just calculated for alpha = -10, 0 and 10 deg, 
                                                           theta = 60, 90, 120 and 130 deg.
                             xt for other angles are also set but they are duplicates,  i.e.
                             xt(alpha =  90 -  20) = xt(alpha =  10); 
                             xt(alpha = -20 - -80) = xt(alpha = -10;)
                             xt(theta = 18, 40) = xt(theta = 60);
                             xt(theta = 149) = xt(theta = 130).

# sigma in new format

* change wrt old format file 
 - add incident angle bin info. at the top of a file in old format.
 - add sigma parameterization mode and no. of params. after that. 
   mode=0: default;
       =1: add 2 params. to describe sigma better near a cell boundary;
       =2: similar to mode=1 but slightly-different parameterization.
* sigma_v2.dat       : Sep.2016 cosmic data (see Apr.12.2017 meeting)
* sigma_v1.dat       : created from sigma_v0.1.dat <- obsolete.


# sigma in old format

The file contains intrinsic space resolution^2 of CDC.

sigma^2 = sigma^2(layer, driftL) [cm^2].

Format: layerID (0-55), param (0-6)
param[0],[1]: primary-ion statistics term
param[2]    : diffusion term
param[3]    : electronics noise term
param[4-6]  : term describes electric distortion near cell boundary

See CDCGeometryPar::getSigma() for detail.

* sigma_simple.dat: assume ideal sigma^2 = 130um^2, indep. of layer or driftL.
* sigma_v0.dat    : obsolete. more realistic one, obtained in beam test.
* sigma_v0.1.dat  : param[6] is added to sigma_v0.dat so that one can change it 
                    externally (it was fixed to the half cell-size - 0.75cm in 
                    the code previously).

# propagation speed

The file contains propagation speed of electric signal along a sense wire.

Format: layerID, speed [cm/ns]

* propspeed_v0.dat: The value observed in Belle-1 CDC is assumed.


# t0

The file contains the constants related to trigger-signal delay (or event time).

Format: layerID (0-55), wireID(0-), t0 [ns]

* t0_v1.dat: Set t0=4900 ns for all wires.
* t0.dat   : Set t0=8192 ns for all wires <- obsolete.

# badwire

The file contains a list of bad wires.

Format: layerID (0-55), wireID(0-)

* badwire_sample.dat: Just a sample file. <- obsolete
* badwire_off.dat   : Empty file; the file for turning-off badwire effect.
* badwire_v1.dat    : contains bad wires known as of Oct.21 2016.
* badwire_v2.dat    : contains bad wires known as of Feburary, 2017.

# time-walk

The file contains coefficients for time-walk (p.h.-dependent timing shift).

Parametrization: time-walk = A/sqrt(ADCcount).

Format: frontend electronics boardID (0-299), A [ns/sqrt(ADCcount)]

* tw_off.dat        : Set A = 0 for all boards.
* tw_crudevalues.dat: Crude As obtained from the beam test in '13. Modified for use in Monte Carlo.

# (mis)alignment

The file contains (mis)alignment parameters w.r.t. displaced geometry (=design+displacement geometry). Misalignment params. are used in digitization; alignment ones in reconstruction.

Format: layerID (0-55), wireID (0-), shift (dx,dy,dz) of sense wire at backward end-plate [cm], (dx,dy,dz) of sense wire at forward end-plate [cm], tension [grW]

* misalignment.dat: assume (dx,dy,dz)=0 and tension=50grW for all wires  -> deleted.
* alignment.dat:  : = misalignment.dat <- obsolete.

* misalignment_v1.dat: assume (dx,dy,dz)=0 and measured tensions for all wires except for Layer#7 47 and Layer#8 47  -> deleted.
* alignment_v1.dat:  : = misalignment_v1.dat <- obsolete

* misalignment_v2.dat: assume (dx,dy,dz)=0 and measured tensions for all wires = 0  -> deleted.
* alignment_v2.dat:  : = misalignment_v2.dat
* alignment_v3.dat: introduced so that this plus displacement_v2.2.1 can reproduce the wire positons which are produced from displacement_v1.1 plus align_201702.dat (by Thanh).

# tentative correction for the difference betw. old design and final design

Only apply to sense wire z-positions.
To be eliminated when the final design is implemented in geometry in future.

Format: layer  backward-deltaz (cm)  forward-deltaz (cm)

deltaz.dat: obsolete (no longer needed).

# displacement  w.r.t. final design

Only apply to sense wire positions.

Format: layerID (0-55), wireID (0-), displacement (dx,dy,dz) of sense wire at backward end-plate [cm], ibid. at forward end-plate [cm]

* deisplacement.dat:  <- obsolete
* deisplacement_v1.dat: tensions added <- obsolete
* deisplacement_v1.1.dat: add corrections for the longer outer wall (by 430um w.r.t. design)
* deisplacement_v2.dat: add corrections (in z-direction only) for the end-plate displacements due to wire tensions.
* deisplacement_v2.1.dat: add other remaining corrections (for endp-late tilt, rotation, etc.). (x,y) as well as z are changed.
* deisplacement_v2.2.1.dat: fixed two bugs in v2.1.

