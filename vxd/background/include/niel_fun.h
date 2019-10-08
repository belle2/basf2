#ifndef _VXD_TNiel_
#define _VXD_TNiel_

#include <string>

/** TNiel - the class providing values for NIEL factors.
 *  The class constructs a spline from a data table, and
 *  evaluates the spline at a desired point.
 */
class TNiel {

private:

  double nielfactor[1710];   /**< factor values, enough to acommodate the longest file.*/
  double E_nielfactor[1710]; /**< particle energies at which the NIEL factor is tabulated.*/
  int niel_N; /**< number of rows in NIEL table */

public:

  /** Constructor takes NIEL table for a particle as input
   * @param FileName String containing the table filename. */
  explicit TNiel(const std::string& FileName);
  /** Get NIEL factor for a given particle energy.
   * @param particle kinetic energy in MeV.
   * @return NIEL factor for a particle with a given energy.
   */
  double getNielFactor(double EMeV);

};

#endif
