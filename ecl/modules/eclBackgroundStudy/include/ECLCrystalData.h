// ECLCrystalData.h : Class
// for obtaining crystal details for a given crystal cell
// An evolved look-up table.
#ifndef ECLCrystalData_h
#define ECLCrystalData_h

namespace Belle2 {

  class ECLCrystalData {
  private :

    //Crystal identifiers
    /** Cell ID of crystal */
    int m_cell_ID;
    /** phi ID of crystal */
    int m_phi_ID;
    /** theta ID of crystal */
    int m_theta_ID;

    //indexes (whatever those mean...)
    /** Index */
    int m_phi_idx;
    /** Index */
    int m_theta_idx;

    //Crystal properties
    /** Crystal Mass */
    float m_mass;
    /** Crystal Volume */
    float m_volume;
    /** Evaluate mass of crystal */
    float EvalMass();

    //Crystal Location
    /** Crystal Position - R*/
    float m_PosR;
    /** Crystal Position - P*/
    float m_PosP;
    /** Crystal Position - X*/
    float m_PosX;
    /** Crystal Position - Y*/
    float m_PosY;
    /** Crystal Position - Z*/
    float m_PosZ;

    /** Set phi value of crystal */
    double EvalPhi();
    /** Set R value of crystal */
    double EvalR();
    /** Set Z value of crystal */
    double EvalZ();
    /** Set X value of crystal */
    double EvalX();
    /** Set Y value of crystal */
    double EvalY();

    /** set theta and phi value of crystal */
    void Mapping(int cid);

  public :


    /** ECL has 16-fold symmetry in phi. Multiply this by 16 to get the total number of crystals at each theta location. */
    static const int Ring[69];
    /** Number of crystals in preceding theta rings (endcaps only) */
    static const int sumPrevious[24];

    const int nECLCrystalTot = 8736; /**< Total number of ECL crystals     */
    const int nECLCrystalBAR = 6624; /**< Number of Barrel ECL crystals    */
    const int nECLCrystalECF = 1152; /**< Number of FWD ECL end-capcrystals*/
    const int nECLCrystalECB =  960; /**< Number of BWD ECL end-capcrystals*/


    /** Barrel crystal mass */
    static const float CrystalMassEndcap[132];
    /** Endcap crystal mass */
    static const float CrystalMassBarrel[46];


    /** Theta ID to theta(deg) */
    static const double theta[69];

    /** EndCapCrystalID to radius (spherical) */
    static const double EndcapRadius[132];

    /** BarrelCrystalID to z-coordinate */
    static const double BarrelZ[46];

    /** Blank Constructor */
    ECLCrystalData();
    /** CrystalID constructor */
    explicit ECLCrystalData(int cid);
    /** theta, phi ID constructor */
    ECLCrystalData(int tid, int pid);
    /** destructor */
    ~ECLCrystalData();
    /** Set values for the crystal */
    void Eval();
    /** return crystal index */
    int GetCrystalIndex();
    /** True if crystal is in endcaps */
    bool IsEndCap();
    /** True if crystal is in barrel */
    bool IsBarrel();
    /** return cell ID */
    int GetCellID();
    /** return cell ID of crystal with given theta and phi id*/
    int GetCellID(int tid, int pid);

    /** get phi value of crystal */
    double GetPhi() { return m_PosP; };
    /** get theta value of crystal */
    inline double GetTheta() { return theta[m_theta_ID]; } ;
    /** get radius of crystal */
    double GetR() { return m_PosR; };
    /** get z position of crystal */
    double GetZ() { return m_PosZ; };
    /** get x position of crystal */
    double GetX() { return m_PosX; };
    /** get y position of crystal */
    double GetY() { return m_PosY; };
    /** get mass of crystal */
    double GetMass() { return m_mass; };
    /** get thetaID of crystal */
    inline int GetThetaID() { return m_theta_ID; } ;
    /** get phiID of crystal */
    inline int GetPhiID() { return m_phi_ID; } ;
    /** get number of crystals in theta ring */
    inline int GetNperThetaID() { return 16 * Ring[m_theta_ID];};


  };
}

#endif
