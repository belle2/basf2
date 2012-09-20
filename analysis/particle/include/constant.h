#ifndef PARTICLE_CLASS_CONSTANT_H
#define PARTICLE_CLASS_CONSTANT_H

const unsigned UNUSABLE = 0;
const unsigned USABLE   = 1;

//MDST_TABLES
const unsigned PC_CHARGED     =  1;
const unsigned PC_GAMMA       =  2;
const unsigned PC_VEE         =  4;
const unsigned PC_KLONG       =  8;
const unsigned PC_PI0         =  16;
const unsigned PC_TRK         =  32;
const unsigned PC_ELID        =  64;
const unsigned PC_MCPARTICLE  =  128;
const unsigned PC_VEE2        =  256;
const unsigned PC_ECL         =  512;
const unsigned PC_ALL        =  PC_CHARGED + PC_GAMMA + PC_VEE + PC_KLONG + PC_PI0 + PC_TRK + PC_ELID + PC_MCPARTICLE + PC_VEE2 + PC_ECL;

#endif /* PARTICLE_CLASS_CONSTANT_H */
