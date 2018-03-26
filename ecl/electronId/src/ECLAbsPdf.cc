#include <ecl/electronId/ECLAbsPdf.h>
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

float ECLAbsPdf::s_energy_unit = 1.0;
float ECLAbsPdf::s_ang_unit = 1.0;

unsigned int ECLAbsPdf::index(const double& p, const double& theta) const
{

  unsigned int ip(n_p_bins - 1);

  // std::cout << "\n\n" << std::endl;
  // std::cout << "Energy unit : " << s_energy_unit << std::endl;
  // std::cout << "N p bins to scan: " << ip << std::endl;
  // std::cout << "p = " << p*s_energy_unit << " [MeV]" << std::endl;
  // std::cout << "abs(theta): " << TMath::Abs(theta*s_ang_unit) << std::endl;

  for (int i(ip); i >= 0; --i) {
    // std::cout << "\tbin low edge - p[" << i << "] = " << p_min[i] << " [MeV]" << std::endl;
    if (p * s_energy_unit > p_min[i]) {
      // std::cout << "\t\tFound!" << std::endl;
      ip = static_cast<unsigned int>(i);
      break;
    }
  }
  unsigned int ith(n_theta_bins - 1);
  for (int i(ith); i >= 0; --i) {
    // std::cout << "\tbin low edge - theta[" << i << "] = " << theta_min[i] << std::endl;
    if (TMath::Abs(theta * s_ang_unit) > theta_min[i]) {
      // std::cout << "\t\tFound!" << std::endl;
      ith = static_cast<unsigned int>(i);
      break;
    }
  }

  // std::cout << "\nIndexes (p,theta) = (" << ip << "_" << ith << ")" << std::endl;
  // std::cout << "\n\n" << std::endl;

  return index(ip, ith);

}

void ECLAbsPdf::init(const ParameterMap& map)
{

  n_theta_bins = map.param("n_theta_bins");
  n_p_bins     = map.param("n_p_bins");

  theta_min = new double[n_theta_bins - 1];
  p_min     = new double[n_p_bins - 1];

  for (unsigned int i(0); i < n_theta_bins; ++i) {
    std::ostringstream nm;
    nm << "theta_" << i << "_min";
    theta_min[i] = map.param(nm.str());
    // std::cout << "theta_" << i << "_min: " << theta_min[i] << std::endl;
  }
  for (unsigned int i(0); i < n_p_bins; ++i) {
    std::ostringstream nm;
    nm << "p_" << i << "_min";
    p_min[i] = map.param(nm.str());
    // std::cout << "p_" << i << "_min: " << p_min[i] << std::endl;
  }
}
