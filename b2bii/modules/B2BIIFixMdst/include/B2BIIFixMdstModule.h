//
// $Id: B2BIIFixMdst.h 11331 2011-07-25 11:58:12Z hitoshi $
//
// $Log$
// Revision 1.16  2005/03/10 14:16:14  hitoshi
// check table sizes
//
// Revision 1.15  2004/04/25 16:33:47  hitoshi
// modified to be able to handle output from low p finder (by kakuno). modified
// Benergy to fetch data from DB (by Shibata).
//
// Revision 1.14  2003/06/24 07:55:15  katayama
// Remived unused parameters
//
// Revision 1.13  2003/06/12 13:25:24  hitoshi
// scale_error implemented last year (by Higuchi; valid up to exp19) is incorporated
//
// Revision 1.12  2003/03/19 05:02:32  hitoshi
// minor change (by Miyabayashi).
//
// Revision 1.11  2002/09/06 03:47:21  hitoshi
// Endcap_MX_layer=11 is now the default (by muon group).
//
// Revision 1.9  2002/04/26 01:25:07  katayama
// New correction from Perers san
//
// Revision 1.8  2002/04/24 00:58:36  hitoshi
// new scale factors for e15 mdst reprocessed with 20020405.
//
// Revision 1.7  2002/04/19 07:24:38  katayama
// Added pi0/vertex, calibdedx
//
// Revision 1.6  2002/04/05 01:19:32  katayama
// templates for ecl_primary_vertex
//
// Revision 1.5  2002/03/31 06:54:39  katayama
// fix_pi0
//
// Revision 1.4  2002/03/30 01:06:05  katayama
// HadronB & L4passed events only
//
// Revision 1.3  2002/03/19 04:25:47  katayama
// Add parameters for scale_momenta, set nominal b energy
//
// Revision 1.2  2002/03/13 02:55:19  katayama
// First version
//
//
#ifndef __FIX_MDST_H__
#define __FIX_MDST_H__

#define HEP_SHORT_NAMES

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include "belle_legacy/belle.h"
#include "belle_legacy/tables/run_info.h"

#include <string>

#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Geometry/Point3D.h"

using namespace Belle;
using namespace CLHEP;
using namespace HepGeom;
typedef Point3D<double> HepPoint3D;

namespace Belle {

  class Mdst_vee2;

}

namespace Belle2 {

  class MuidProb;

//
// Declaration of class B2BIIFixMdst
//

  class B2BIIFixMdstModule : public Belle2::Module {

    static const int m_muid_version = 100;      // version number of muid part

  public:
    /**
     * Constructor
     */
    B2BIIFixMdstModule();

    /**
     * Initialize the module.
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     */
    virtual void beginRun();

    /**
     * Called for each event.
     */
    virtual void event();

    /**
     * Terminates the module.
     */
    virtual void terminate();

    static double get_Benergy()
    {
      return s_benergy_value;
    };

    static double get_pi0resol(double p, double theta, char* side,
                               bool mcdata, int exp, int option)
    {
      return pi0resol(p, theta, side, mcdata, exp, option);
    };

  public: // BASF Parameters

    int m_scale_momenta;
    float m_scale_momenta_scale_data;
    float m_scale_momenta_scale_mc;
    int m_scale_momenta_mode;
    int m_scale_error;
    int m_smear_trk;
    std::string m_scale_error_dataset_name;
    int m_add_or_remove_extra_trk_vee2;
    int m_correct_ecl;
    int m_correct_ecl_option;
    int m_correct_ecl_version;
    int m_correct_ecl_primary_vertex;
    int m_correct_ecl_5s; // For run indep. MC exp.61 5S runs.
    int m_make_pi0;
    int m_make_pi0_option;
    double m_make_pi0_lower_limit;
    double m_make_pi0_upper_limit;
    int m_make_pi0_primary_vertex;
    int m_benergy;
    int m_benergy_db;
    int m_benergy_mcdb;
    static double s_benergy_value;
    int m_good_event;
    int m_hadron_a_only;
    int m_hadron_b_only;
    int m_l4passed_only;
    int m_calib_dedx;
    int m_shift_tof_mode;
    int m_table_size;
    int m_limit_mdst_ecl_trk;
    int m_limit_mdst_klm_cluster_hit;
    int m_reprocess_version;
    int m_reprocess_version_specified;
    int m_5Srun;
    int m_check_version_consistency;
    /**@name Muid user parameters (set via basf) */
    //@{
    /// parameter Endcap_MX_layer: Max layer # of KLM endcap (default=11)
    ///   0 ==> use same value as was used by muid\_set and muid\_dec during DST prod
    ///  11 ==> discard two outermost layers of KLM endcap
    ///  13 ==> use all layers of KLM endcap, if possible
    int m_eklm_max_layer;
    /// parameter UseECL:  Use (1) or don't use (0) ECL in muid (default=0)
    int m_use_ecl;
    /// parameter ExpNo:  Experiment number for muid (default=0)
    ///   0 ==> use experiment number stored in data file
    ///  >0 ==> override experiment number with this value
    ///  <0 ==> disable muid code within fix\_mdst
    int m_expno;
    //@}

  protected:

  private:

    int  m_old_expno;          // most recently used experiment # in muid
    int  m_mapped_expno;       // mapped value of m_old_exp_no

    MuidProb* m_muonprob;      // Pointer to muons' prob-density object
    MuidProb* m_pionprob;      // Pointer to pions' prob-density object
    MuidProb* m_kaonprob;      // Pointer to kaons' prob-density object

    bool good_event();

    int get_reprocess_version();

    void scale_momenta(float scale_data = 1.0, float scale_mc = 1.0, int mode = 0);

    void scale_momenta_set(const int, const int, const int, double&);
    void scale_momenta_set_v1(const int, const int, const int, double&);
    void scale_momenta_set_v2(const int, const int, const int, double&);

    double vee_mass_nofit(const Mdst_vee2& vee2, float scale = 1.0);

    //  Author:  J.Tanaka (+T.Higuchi/2002,2003)
    void scale_error(const int message_level = 0);
    //  void scale_error(void);

    void smear_trk();

    // Author: H.Kakuno
    int add_extra_trk_vee2();
    int remove_extra_trk_vee2();

    //  Author:  Miyabayashi
    void make_pi0(int, double, double);
    void correct_ecl(int, int);

    int set_primary_vertex(HepPoint3D& v, HepSymMatrix& ve);

    void make_pi0_primary_vertex(int, double, double, const HepPoint3D&,
                                 const HepSymMatrix&);
    void correct_ecl_primary_vertex(const HepPoint3D&,
                                    const HepSymMatrix&);
    // Treat pi0 mass width as a func. of pi0 momentum.
    static double pi0resol(double, double, char*, bool, int, int);

    //  Author:  T. Matsumoto
    //  based on dcpvrare_ml:0127, interface is modified by M. Nakao
    //

    double Benergy(int expnum = 0, int runnum = 0);

    // Author: Kakuno
    void fix_pi0_brecon(void);
    // Author: Mike Peters
    void shift_tof(const int mode);
    void shift_tof_set(const int expno, const int runno,
                       const int mode, const int im,
                       const double pmom, const double sgn,
                       double& shift);
    // Author: Leo Piilonen
    void Muid_init(void);
    void Muid_term(void);
    void Muid_event(void);
    void Muid_begin_run(const int, const int, const int);
    void Muid_end_run(void);

  };


  class MuidProb {

  public:

    /**@name MuidProb methods: */
    //@{
    /// Constructor
    MuidProb(const char*, int&);
    /// Destructor
    ~MuidProb() {};
    /// Compute probability density
    double prob(int, int, int, int, double) const;
    /// Compute probability density for range
    double probRange(int, int, int, int) const;
    /// Compute probability density for reduced chi-squared
    double probRchisq(int, int, int, double) const;
    /// Compute probability density for ECL energy deposit
    double probECL(double, double) const;
    /// Read in probability density functions from database.
    void readDB(const char*, int&);
    //@}

  private:

    // Array sizes as stored in input file

    static const int kRange  = 16;
    static const int kRchisq = 50;
    static const int kEEcl   = 100;
    static const int kPTrk   = 50;

    // Overflow value of reduced chi-squared, ECL energy, CDC momentum

    static const double kRchisqMax;
    static const double kEEclMax;
    static const double kPTrkMax;

    // Probability density arrays for range, reduced chi**2, and ECL energy
    // (last one sliced by tracdk momentum in 100 MeV/c bins).
    // One extra bin in chi-squared and ECL-energy distributions for overflows.
    // For reduced chi-squared and ECL-energy, three extra arrays are stored for
    // spline interpolation to eliminate binning artifacts.

    double fRange[2][4][15][kRange];      // range pdf
    double fRchisq[2][4][kRchisq + 1]; // reduced chi-squared pdf
    double fRchisqN[2][4][15];    // non-overflow normalization
    double fRchisqD1[2][4][kRchisq + 1]; // first derivatives of above
    double fRchisqD2[2][4][kRchisq + 1]; // second derivatives of above
    double fRchisqD3[2][4][kRchisq + 1]; // third derivatives of above
    double fEEcl[kPTrk][kEEcl + 1];       // ECL energy pdf
    double fEEclD1[kPTrk][kEEcl + 1];     // first derivatives of above
    double fEEclD2[kPTrk][kEEcl + 1];     // second derivatives of above
    double fEEclD3[kPTrk][kEEcl + 1];     // third derivatives of above

    // private member functions

    void spline(int, double, double*, double*, double*, double*);

  };

} // namespace Belle
#endif /* __FIX_MDST_H__ */

