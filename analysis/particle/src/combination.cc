#include "analysis/particle/combination.h"

using namespace Belle2;

/* bool
   checkSame(const Particle &p1,
   const Particle &p2){
   if(p1.mdstCharged() && p2.mdstCharged() &&
   p1.mdstCharged().get_ID() == p2.mdstCharged().get_ID())return true;
   return false;
   } */

bool
checkSame(const Particle& p1,
          const Particle& p2)
{
  // same     --> return true;
  // not same --> return false;
  if (p1.nChildren() == 0 &&
      p2.nChildren() == 0) {
    /* p1 and p2 have no children */
    return p1.relation().isIdenticalWith(p2.relation());
  } else if (p1.nChildren() > 0 &&
             p2.nChildren() == 0) {
    /* p1 have children and no p2 children */
    for (unsigned i1 = 0; i1 < p1.nChildren(); ++i1)
      if (checkSame(p1.child(i1), p2))return true;
    return false;
  } else if (p1.nChildren() == 0 &&
             p2.nChildren() > 0) {
    /* p2 have children and no p1 children */
    for (unsigned i2 = 0; i2 < p2.nChildren(); ++i2)
      if (checkSame(p1, p2.child(i2)))return true;
    return false;
  } else {
    /* p1 and p2 both have children */
    for (unsigned i1 = 0; i1 < p1.nChildren(); ++i1)
      for (unsigned i2 = 0; i2 < p2.nChildren(); ++i2)
        if (checkSame(p1.child(i1), p2.child(i2)))return true;
    return false;
  }
}

/************************/
/* 2 Bodies Combination */
/************************/
//...p1 != p2
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2);
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        const double& massL,
                        const double& massR);
//...p1 == p2
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2);
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        const double& massL,
                        const double& massR);
/************************/
/* 3 Bodies Combination */
/************************/
//...p1 != p2, p1 != p3, p2 != p3
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3);
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        const double& massL,
                        const double& massR);
//...p1 == p2, p1 != p3
//...p1 == p3, p1 != p2
//...p2 == p3, p2 != p1
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3);
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        const double& massL,
                        const double& massR);
//...p1 == p2 == p3
void combination_type03(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3);
void combination_type03(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        const double& massL,
                        const double& massR);
/************************/
/* 4 Bodies Combination */
/************************/
//...p1 != p2, p1 != p3, p1 != p4, p2 != p3, p2 != p4, p3 != p4
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4);
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        const double& massL,
                        const double& massR);
//...p1 == p2, p1 != p3, p1 != p4, p3 != p4
//...p1 == p3, p1 != p2, p1 != p4, p2 != p4
//...p1 == p4, p1 != p2, p1 != p3, p2 != p3
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4);
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        const double& massL,
                        const double& massR);
//...p1 == p2, p3 == p4, p1 != p3
//...p1 == p3, p2 == p4, p1 != p2
//...p1 == p4, p2 == p3, p1 != p2
void combination_type03(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4);
void combination_type03(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        const double& massL,
                        const double& massR);
//...p1 == p2 == p3, p1 != p4
//...p1 == p2 == p4, p1 != p3
//...p1 == p3 == p4, p1 != p2
//...p2 == p3 == p4, p2 != p1
void combination_type04(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4);
void combination_type04(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        const double& massL,
                        const double& massR);
//...p1 == p2 == p3 == p4
void combination_type05(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4);
void combination_type05(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        const double& massL,
                        const double& massR);
/************************/
/* 5 Bodies Combination */
/************************/
//...p1 != p2, p1 != p3, p1 != p4, p1 != p5, p2 != p3, p2 != p4, p2 != p5, p3 != p4, p3 != p5, p4 != p5
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5);
void combination_type01(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5,
                        const double& massL,
                        const double& massR);
//...p1 == p2, p1 != p3, p1 != p4, p1 != p5, p3 != p4, p3 != p5, p4 != p5
//...p1 == p3, p1 != p3, p1 != p4, p1 != p5, p3 != p4, p3 != p5, p4 != p5
//...p1 == p4, p1 != p3, p1 != p4, p1 != p5, p3 != p4, p3 != p5, p4 != p5
//...p1 == p5, p1 != p3, p1 != p4, p1 != p5, p3 != p4, p3 != p5, p4 != p5
//...
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5);
void combination_type02(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5,
                        const double& massL,
                        const double& massR);
//...p1 == p2, p3 == p4, p1 != p3, p1 != p5, p3 != p5
//...
void combination_type03(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5);
void combination_type03(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5,
                        const double& massL,
                        const double& massR);
//...p1 == p2 == p3, p4 != p5, p1 != p4
//...
void combination_type04(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5);
void combination_type04(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5,
                        const double& massL,
                        const double& massR);
//...p1 == p2 == p3, p4 == p5, p1 != p4
//...
void combination_type05(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5);
void combination_type05(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5,
                        const double& massL,
                        const double& massR);
//...p1 == p2 == p3 == p4, p1 != p5
//...
void combination_type06(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5);
void combination_type06(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5,
                        const double& massL,
                        const double& massR);
//...p1 == p2 == p3 == p4 == p5
void combination_type07(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5);
void combination_type07(std::vector<Particle> &new_p,
                        const Ptype& ptype,
                        std::vector<Particle> &p1,
                        std::vector<Particle> &p2,
                        std::vector<Particle> &p3,
                        std::vector<Particle> &p4,
                        std::vector<Particle> &p5,
                        const double& massL,
                        const double& massR);
/*************/
/* functions */
/*************/


/************/
/* 2 bodies */
/************/
void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2)
{
  if (&p1 != &p2) {
    combination_type01(new_p, ptype, p1, p2);
  } else {
    combination_type02(new_p, ptype, p1, p2);
  }
}

void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            const double& mass_width)
{
  if (&p1 != &p2) {
    combination_type01(new_p, ptype, p1, p2, mass_width, mass_width);
  } else {
    combination_type02(new_p, ptype, p1, p2, mass_width, mass_width);
  }
}

void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            const double& massL,
            const double& massR)
{
  if (&p1 != &p2) {
    combination_type01(new_p, ptype, p1, p2, massL, massR);
  } else {
    combination_type02(new_p, ptype, p1, p2, massL, massR);
  }
}

void
combination_nocut(std::vector<Particle> &new_p,
                  const Ptype& ptype,
                  Particle& p1,
                  Particle& p2)
{
  if (checkSame(p1, p2))return;
  Particle cand(p1.momentum().p() + p2.momentum().p(), ptype);
  cand.relation().append(p1);
  cand.relation().append(p2);
  new_p.push_back(cand);
}

void
combination_cut(std::vector<Particle> &new_p,
                const Ptype& ptype,
                Particle& p1,
                Particle& p2,
                const double& massL,
                const double& massR)
{
  if (checkSame(p1, p2))return;
  //...checks mass
  double mass = (p1.momentum().p() + p2.momentum().p()).mag();
  double new_mass = ptype.mass();
  if (new_mass - massL <= mass && mass <= new_mass + massR) {
    Particle cand(p1.momentum().p() + p2.momentum().p(), ptype);
    cand.relation().append(p1);
    cand.relation().append(p2);
    new_p.push_back(cand);
  }
}

void
combination_type01(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &p2)
{
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end(); ++i) {
    for (std::vector<Particle>::iterator j = p2.begin();
         j != p2.end(); ++j) {
      combination_nocut(new_p, ptype, *i, *j);
    }
  }
}

void
combination_type01(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &p2,
                   const double& massL,
                   const double& massR)
{
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end(); ++i) {
    for (std::vector<Particle>::iterator j = p2.begin();
         j != p2.end(); ++j) {
      combination_cut(new_p, ptype, *i, *j, massL, massR);
    }
  }
}

void
combination_type02(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &)
{
  if (p1.size() < 2)return;
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end() - 1; ++i) {
    for (std::vector<Particle>::iterator j = i + 1;
         j != p1.end(); ++j) {
      combination_nocut(new_p, ptype, *i, *j);
    }
  }
}

void
combination_type02(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &,
                   const double& massL,
                   const double& massR)
{
  if (p1.size() < 2)return;
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end() - 1; ++i) {
    for (std::vector<Particle>::iterator j = i + 1;
         j != p1.end(); ++j) {
      combination_cut(new_p, ptype, *i, *j, massL, massR);
    }
  }
}

/************/
/* 3 bodies */
/************/
void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            std::vector<Particle> &p3)
{
  if (&p1 != &p2 &&
      &p1 != &p3 &&
      &p2 != &p3) {
    combination_type01(new_p, ptype, p1, p2, p3);
  } else if ((&p1 == &p2) &&
             (&p1 != &p3)) {
    combination_type02(new_p, ptype, p1, p2, p3);
  } else if ((&p1 == &p3) &&
             (&p1 != &p2)) {
    combination_type02(new_p, ptype, p1, p3, p2);
  } else if ((&p2 == &p3) &&
             (&p2 != &p1)) {
    combination_type02(new_p, ptype, p2, p3, p1);
  } else {
    //dout(Debugout::ERR,"combination") << "NOT Support in 3 bodies" << std::endl;
    std::cout << "NOT Support in 3 bodies" << std::endl;
  }
}

void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            std::vector<Particle> &p3,
            const double& mass_width)
{
  if (&p1 != &p2 &&
      &p1 != &p3 &&
      &p2 != &p3) {
    combination_type01(new_p, ptype, p1, p2, p3, mass_width, mass_width);
  } else if ((&p1 == &p2) &&
             (&p1 != &p3)) {
    combination_type02(new_p, ptype, p1, p2, p3, mass_width, mass_width);
  } else if ((&p1 == &p3) &&
             (&p1 != &p2)) {
    combination_type02(new_p, ptype, p1, p3, p2, mass_width, mass_width);
  } else if ((&p2 == &p3) &&
             (&p2 != &p1)) {
    combination_type02(new_p, ptype, p2, p3, p1, mass_width, mass_width);
  } else {
    //dout(Debugout::ERR,"combination") << "NOT Support in 3 bodies" << std::endl;
    std::cout << "NOT Support in 3 bodies" << std::endl;
  }
}

void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            std::vector<Particle> &p3,
            const double& massL,
            const double& massR)
{
  if (&p1 != &p2 &&
      &p1 != &p3 &&
      &p2 != &p3) {
    combination_type01(new_p, ptype, p1, p2, p3, massL, massR);
  } else if ((&p1 == &p2) &&
             (&p1 != &p3)) {
    combination_type02(new_p, ptype, p1, p2, p3, massL, massR);
  } else if ((&p1 == &p3) &&
             (&p1 != &p2)) {
    combination_type02(new_p, ptype, p1, p3, p2, massL, massR);
  } else if ((&p2 == &p3) &&
             (&p2 != &p1)) {
    combination_type02(new_p, ptype, p2, p3, p1, massL, massR);
  } else {
    //dout(Debugout::ERR,"combination") << "NOT Support in 3 bodies" << std::endl;
    std::cout << "NOT Support in 3 bodies" << std::endl;
  }
}

void
combination_nocut(std::vector<Particle> &new_p,
                  const Ptype& ptype,
                  Particle& p1,
                  Particle& p2,
                  Particle& p3)
{
  if (checkSame(p1, p2))return;
  if (checkSame(p1, p3))return;
  if (checkSame(p2, p3))return;
  Particle cand(p1.momentum().p() + p2.momentum().p() + p3.momentum().p(), ptype);
  cand.relation().append(p1);
  cand.relation().append(p2);
  cand.relation().append(p3);
  new_p.push_back(cand);
}

void
combination_cut(std::vector<Particle> &new_p,
                const Ptype& ptype,
                Particle& p1,
                Particle& p2,
                Particle& p3,
                const double& massL,
                const double& massR)
{
  if (checkSame(p1, p2))return;
  if (checkSame(p1, p3))return;
  if (checkSame(p2, p3))return;
  //...checks mass
  double mass = (p1.momentum().p() + p2.momentum().p() + p3.momentum().p()).mag();
  double new_mass = ptype.mass();
  if (new_mass - massL <= mass && mass <= new_mass + massR) {
    Particle cand(p1.momentum().p() + p2.momentum().p() + p3.momentum().p(), ptype);
    cand.relation().append(p1);
    cand.relation().append(p2);
    cand.relation().append(p3);
    new_p.push_back(cand);
  }
}

void
combination_type01(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &p2,
                   std::vector<Particle> &p3)
{
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end(); ++i) {
    for (std::vector<Particle>::iterator j = p2.begin();
         j != p2.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        combination_nocut(new_p, ptype, *i, *j, *k);
      }
    }
  }
}

void
combination_type01(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &p2,
                   std::vector<Particle> &p3,
                   const double& massL,
                   const double& massR)
{
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end(); ++i) {
    for (std::vector<Particle>::iterator j = p2.begin();
         j != p2.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        combination_cut(new_p, ptype, *i, *j, *k, massL, massR);
      }
    }
  }
}

void
combination_type02(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &,
                   std::vector<Particle> &p3)
{
  if (p1.size() < 2)return;
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end() - 1; ++i) {
    for (std::vector<Particle>::iterator j = i + 1;
         j != p1.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        combination_nocut(new_p, ptype, *i, *j, *k);
      }
    }
  }
}

void
combination_type02(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &,
                   std::vector<Particle> &p3,
                   const double& massL,
                   const double& massR)
{
  if (p1.size() < 2)return;
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end() - 1; ++i) {
    for (std::vector<Particle>::iterator j = i + 1;
         j != p1.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        combination_cut(new_p, ptype, *i, *j, *k, massL, massR);
      }
    }
  }
}

/************/
/* 4 bodies */
/************/
void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            std::vector<Particle> &p3,
            std::vector<Particle> &p4)
{
  if (&p1 != &p2 &&
      &p1 != &p3 &&
      &p1 != &p4 &&
      &p2 != &p3 &&
      &p2 != &p4 &&
      &p3 != &p4) {
    combination_type01(new_p, ptype, p1, p2, p3, p4);
  } else if ((&p1 == &p2) &&
             (&p1 != &p3) &&
             (&p1 != &p4) &&
             (&p3 != &p4)) {
    combination_type02(new_p, ptype, p1, p2, p3, p4);
  } else if ((&p1 == &p3) &&
             (&p1 != &p2) &&
             (&p1 != &p4) &&
             (&p2 != &p4)) {
    combination_type02(new_p, ptype, p1, p3, p2, p4);
  } else if ((&p1 == &p4) &&
             (&p1 != &p2) &&
             (&p1 != &p3) &&
             (&p2 != &p3)) {
    combination_type02(new_p, ptype, p1, p4, p2, p3);
  } else if ((&p2 == &p3) &&
             (&p2 != &p1) &&
             (&p2 != &p4) &&
             (&p1 != &p4)) {
    combination_type02(new_p, ptype, p2, p3, p1, p4);
  } else if ((&p2 == &p4) &&
             (&p2 != &p1) &&
             (&p2 != &p3) &&
             (&p1 != &p3)) {
    combination_type02(new_p, ptype, p2, p4, p1, p3);
  } else if ((&p3 == &p4) &&
             (&p3 != &p1) &&
             (&p3 != &p2) &&
             (&p1 != &p2)) {
    combination_type02(new_p, ptype, p3, p4, p1, p2);
  } else {
    //dout(Debugout::ERR,"combination") << "NOT Support in 4 bodies" << std::endl;
    std::cout << "NOT Support in 4 bodies" << std::endl;
  }
}

void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            std::vector<Particle> &p3,
            std::vector<Particle> &p4,
            const double& mass_width)
{
  if (&p1 != &p2 &&
      &p1 != &p3 &&
      &p1 != &p4 &&
      &p2 != &p3 &&
      &p2 != &p4 &&
      &p3 != &p4) {
    combination_type01(new_p, ptype, p1, p2, p3, p4, mass_width, mass_width);
  } else if ((&p1 == &p2) &&
             (&p1 != &p3) &&
             (&p1 != &p4) &&
             (&p3 != &p4)) {
    combination_type02(new_p, ptype, p1, p2, p3, p4, mass_width, mass_width);
  } else if ((&p1 == &p3) &&
             (&p1 != &p2) &&
             (&p1 != &p4) &&
             (&p2 != &p4)) {
    combination_type02(new_p, ptype, p1, p3, p2, p4, mass_width, mass_width);
  } else if ((&p1 == &p4) &&
             (&p1 != &p2) &&
             (&p1 != &p3) &&
             (&p2 != &p3)) {
    combination_type02(new_p, ptype, p1, p4, p2, p3, mass_width, mass_width);
  } else if ((&p2 == &p3) &&
             (&p2 != &p1) &&
             (&p2 != &p4) &&
             (&p1 != &p4)) {
    combination_type02(new_p, ptype, p2, p3, p1, p4, mass_width, mass_width);
  } else if ((&p2 == &p4) &&
             (&p2 != &p1) &&
             (&p2 != &p3) &&
             (&p1 != &p3)) {
    combination_type02(new_p, ptype, p2, p4, p1, p3, mass_width, mass_width);
  } else if ((&p3 == &p4) &&
             (&p3 != &p1) &&
             (&p3 != &p2) &&
             (&p1 != &p2)) {
    combination_type02(new_p, ptype, p3, p4, p1, p2, mass_width, mass_width);
  } else {
    //dout(Debugout::ERR,"combination") << "NOT Support in 4 bodies" << std::endl;
    std::cout << "NOT Support in 4 bodies" << std::endl;
  }
}

void
combination(std::vector<Particle> &new_p,
            const Ptype& ptype,
            std::vector<Particle> &p1,
            std::vector<Particle> &p2,
            std::vector<Particle> &p3,
            std::vector<Particle> &p4,
            const double& massL,
            const double& massR)
{
  if (&p1 != &p2 &&
      &p1 != &p3 &&
      &p1 != &p4 &&
      &p2 != &p3 &&
      &p2 != &p4 &&
      &p3 != &p4) {
    combination_type01(new_p, ptype, p1, p2, p3, p4, massL, massR);
  } else if ((&p1 == &p2) &&
             (&p1 != &p3) &&
             (&p1 != &p4) &&
             (&p3 != &p4)) {
    combination_type02(new_p, ptype, p1, p2, p3, p4, massL, massR);
  } else if ((&p1 == &p3) &&
             (&p1 != &p2) &&
             (&p1 != &p4) &&
             (&p2 != &p4)) {
    combination_type02(new_p, ptype, p1, p3, p2, p4, massL, massR);
  } else if ((&p1 == &p4) &&
             (&p1 != &p2) &&
             (&p1 != &p3) &&
             (&p2 != &p3)) {
    combination_type02(new_p, ptype, p1, p4, p2, p3, massL, massR);
  } else if ((&p2 == &p3) &&
             (&p2 != &p1) &&
             (&p2 != &p4) &&
             (&p1 != &p4)) {
    combination_type02(new_p, ptype, p2, p3, p1, p4, massL, massR);
  } else if ((&p2 == &p4) &&
             (&p2 != &p1) &&
             (&p2 != &p3) &&
             (&p1 != &p3)) {
    combination_type02(new_p, ptype, p2, p4, p1, p3, massL, massR);
  } else if ((&p3 == &p4) &&
             (&p3 != &p1) &&
             (&p3 != &p2) &&
             (&p1 != &p2)) {
    combination_type02(new_p, ptype, p3, p4, p1, p2, massL, massR);
  } else {
    //dout(Debugout::ERR,"combination") << "NOT Support in 4 bodies" << std::endl;
    std::cout << "NOT Support in 4 bodies" << std::endl;
  }
}

void
combination_nocut(std::vector<Particle> &new_p,
                  const Ptype& ptype,
                  Particle& p1,
                  Particle& p2,
                  Particle& p3,
                  Particle& p4)
{
  if (checkSame(p1, p2))return;
  if (checkSame(p1, p3))return;
  if (checkSame(p1, p4))return;
  if (checkSame(p2, p3))return;
  if (checkSame(p2, p4))return;
  if (checkSame(p3, p4))return;
  Particle cand(p1.momentum().p() + p2.momentum().p() + p3.momentum().p() + p4.momentum().p(), ptype);
  cand.relation().append(p1);
  cand.relation().append(p2);
  cand.relation().append(p3);
  cand.relation().append(p4);
  new_p.push_back(cand);
}

void
combination_cut(std::vector<Particle> &new_p,
                const Ptype& ptype,
                Particle& p1,
                Particle& p2,
                Particle& p3,
                Particle& p4,
                const double& massL,
                const double& massR)
{
  if (checkSame(p1, p2))return;
  if (checkSame(p1, p3))return;
  if (checkSame(p1, p4))return;
  if (checkSame(p2, p3))return;
  if (checkSame(p2, p4))return;
  if (checkSame(p3, p4))return;
  //...checks mass
  double mass = (p1.momentum().p() + p2.momentum().p() + p3.momentum().p() + p4.momentum().p()).mag();
  double new_mass = ptype.mass();
  if (new_mass - massL <= mass && mass <= new_mass + massR) {
    Particle cand(p1.momentum().p() + p2.momentum().p() + p3.momentum().p() + p4.momentum().p(), ptype);
    cand.relation().append(p1);
    cand.relation().append(p2);
    cand.relation().append(p3);
    cand.relation().append(p4);
    new_p.push_back(cand);
    return;
  } else {
    return;
  }
}

void
combination_type01(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &p2,
                   std::vector<Particle> &p3,
                   std::vector<Particle> &p4)
{
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end(); ++i) {
    for (std::vector<Particle>::iterator j = p2.begin();
         j != p2.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        for (std::vector<Particle>::iterator l = p4.begin();
             l != p4.end(); ++l) {
          combination_nocut(new_p, ptype, *i, *j, *k, *l);
        }
      }
    }
  }
}

void
combination_type01(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &p2,
                   std::vector<Particle> &p3,
                   std::vector<Particle> &p4,
                   const double& massL,
                   const double& massR)
{
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end(); ++i) {
    for (std::vector<Particle>::iterator j = p2.begin();
         j != p2.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        for (std::vector<Particle>::iterator l = p4.begin();
             l != p4.end(); ++l) {
          combination_cut(new_p, ptype, *i, *j, *k, *l, massL, massR);
        }
      }
    }
  }
}

void
combination_type02(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &,
                   std::vector<Particle> &p3,
                   std::vector<Particle> &p4)
{
  if (p1.size() < 2)return;
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end() - 1; ++i) {
    for (std::vector<Particle>::iterator j = i + 1;
         j != p1.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        for (std::vector<Particle>::iterator l = p4.begin();
             l != p4.end(); ++l) {
          combination_nocut(new_p, ptype, *i, *j, *k, *l);
        }
      }
    }
  }
}

void
combination_type02(std::vector<Particle> &new_p,
                   const Ptype& ptype,
                   std::vector<Particle> &p1,
                   std::vector<Particle> &,
                   std::vector<Particle> &p3,
                   std::vector<Particle> &p4,
                   const double& massL,
                   const double& massR)
{
  if (p1.size() < 2)return;
  for (std::vector<Particle>::iterator i = p1.begin();
       i != p1.end() - 1; ++i) {
    for (std::vector<Particle>::iterator j = i + 1;
         j != p1.end(); ++j) {
      for (std::vector<Particle>::iterator k = p3.begin();
           k != p3.end(); ++k) {
        for (std::vector<Particle>::iterator l = p4.begin();
             l != p4.end(); ++l) {
          combination_cut(new_p, ptype, *i, *j, *k, *l, massL, massR);
        }
      }
    }
  }
}

/************/
/* 5 Bodies */
/************/
void
combination(std::vector<Particle> &,
            const Ptype&,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &)
{
  //dout(Debugout::ERR,"combination") << "NOT Support in 5 bodies" << std::endl;
  std::cout << "NOT Support in 5 bodies" << std::endl;
}

void
combination(std::vector<Particle> &,
            const Ptype&,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            const double&)
{
  //dout(Debugout::ERR,"combination") << "NOT Support in 5 bodies" << std::endl;
  std::cout << "NOT Support in 5 bodies" << std::endl;
}

void
combination(std::vector<Particle> &,
            const Ptype&,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            std::vector<Particle> &,
            const double&,
            const double&)
{
  //dout(Debugout::ERR,"combination") << "NOT Support in 5 bodies" << std::endl;
  std::cout << "NOT Support in 5 bodies" << std::endl;
}
#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
