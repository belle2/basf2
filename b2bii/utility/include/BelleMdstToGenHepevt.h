// ****************************************************************
// Belle MDST objects -> MC Particle relations
//
// Original file: ${BELLE_TOP_DIR}/src/anal/mdst/mdst/mdst.h
//
// Ported by: A. Zupanc (March 2015)
//
// ****************************************************************

class Mdst_charged;
class Mdst_ecl;
class Mdst_gamma;
class Gen_hepevt;
class Mdst_trk;

/**
 * Returns reference to MC matched Gen_hepevt object
 */
const Gen_hepevt& get_hepevt(const Mdst_trk&, int ith = 0);

/**
 * Returns reference to MC matched Gen_hepevt object
 */
const Gen_hepevt& get_hepevt(const Mdst_charged&, int ith = 0);

/**
 * Returns reference to MC matched Gen_hepevt object
 *
 * returns correponding hepevt(This hepevt might be made at inside of ECL).
 * the following example will return what most of you want!
 * Mdst_ecl & e = ...
 * const Gen_hepevt & h(gen_level(get_hepevt(e)));
 *
 */
const Gen_hepevt& get_hepevt(const Mdst_ecl&, int ith = 0);

/**
 * Returns reference to MC matched Gen_hepevt object
 *
 * returns correponding hepevt(This hepevt might be made at inside of ECL).
 * the following example will return what most of you want!
 * Mdst_ecl & e = ...
 * const Gen_hepevt & h(gen_level(get_hepevt(e)));
 *
 */
const Gen_hepevt& get_hepevt(const Mdst_gamma&, int ith = 0);

/**
 * trace back to mother hepevt which has isthep>0 or
 * isthep =-10(in case of daughters of Ks/Lambda)
 */
const Gen_hepevt& gen_level(const Gen_hepevt&);

