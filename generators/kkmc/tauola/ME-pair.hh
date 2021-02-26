extern "C" void eepi_(const double& m_dark, const double& dark_width, const double& Mlep, const double& Mpion,
		      const double *pt, const double *pn, const double *p1, const double *p2, const double *p3, double &amplit, double *hv,int& iflag);

extern "C" void eemu_(const double& m_dark, const double& dark_width, const double& Mtau,
		      const double *pt, const double *pn, const double *pe1, const double *pe2, const double *pmu, const double *pnu_mu, double &amplit, double *hv);

double nunul_(const double& Mtau, const double *pl,const double *pnu,const double *pnl,double ak0,double *hv);

static int e_all = 0;
static int e_neg = 0;
static int m_all = 0;
static int m_neg = 0;
