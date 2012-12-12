#ifndef BPCU_COMBINATION_FUNCTIONS_PC_H
#define BPCU_COMBINATION_FUNCTIONS_PC_H

#include "analysis/particle/Particle.h"
#include "analysis/particle/constant.h"

using namespace Belle1;

/// Utilies for Making Combination of Particle.
///
/// Note 1: Only for particle constructed by MdstCharged.
///
/// mass_width -->
/// nominal mass - mass_width < MASS < nominal mass + mass_width
///
/// massL, massR -->
/// nominal mass - massL < MASS < nominal mass + massR
///
/// nominal mass is defined by ptype.mass().
///

bool checkSame(const Particle& p1,
               const Particle& p2);

/**********************/
/* 2 Boby Combination */
/**********************/
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 const double& mass_width);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 const double& massL,
                 const double& massR);

/**********************/
/* 3 Boby Combination */
/**********************/
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 const double& mass_width);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 const double& massL,
                 const double& massR);

/**********************/
/* 4 Boby Combination */
/**********************/
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 std::vector<Particle> &p4);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 std::vector<Particle> &p4,
                 const double& mass_width);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 std::vector<Particle> &p4,
                 const double& massL,
                 const double& massR);

/**********************/
/* 5 Boby Combination */
/**********************/
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 std::vector<Particle> &p4,
                 std::vector<Particle> &p5);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 std::vector<Particle> &p4,
                 std::vector<Particle> &p5,
                 const double& mass_width);
void combination(std::vector<Particle> &new_p,
                 const Ptype& ptype,
                 std::vector<Particle> &p1,
                 std::vector<Particle> &p2,
                 std::vector<Particle> &p3,
                 std::vector<Particle> &p4,
                 std::vector<Particle> &p5,
                 const double& massL,
                 const double& massR);

#endif /* BPCU_COMBINATION_FUNCTIONS_PC_H */
