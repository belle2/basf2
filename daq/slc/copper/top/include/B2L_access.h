#ifndef __B2LACCESS__
#define __B2LACCESS__

extern const int SCROD_UNDEFINED;
extern const int CARRIER_UNDEFINED;
extern const unsigned TIMEOUT;

#define HSLBMAX 4

extern int errorcountReadBack[HSLBMAX];
extern int errorcountTimeOut[HSLBMAX];
extern int errorcountDeadBeef[HSLBMAX];
extern int transactioncount[HSLBMAX];

const unsigned ReadIgnoreRegs[] = {0x2800, 0x2C00, 0x3000, 0x3400,   // IRSX direct. carrier 0
                                   0x4800, 0x4C00, 0x5000, 0x5400,   // IRSX direct, carrier 1
                                   0x6800, 0x6C00, 0x7000, 0x7400,   // IRSX direct, carrier 2
                                   0x8800, 0x8C00, 0x9000, 0x9400,   // IRSX direct, carrier 3
                                   0x0206, 0x0207, 0x04AA, 0x04B0,   // Misc. SCROD write-only
                                   0x04FF,
                                   0x0600, 0x0800, 0x0A00, 0x0C00,   // Misc. PGP registers
                                   0x0601, 0x0801, 0x0A01, 0x0C01,
                                   0x0602, 0x0802, 0x0A02, 0x0C02,
                                   0x2206, 0x4206, 0x6206, 0x8206,   // Misc. carrier write-only
                                   0x2207, 0x4207, 0x6207, 0x8207,
                                   0x24AA, 0x44AA, 0x64AA, 0x84AA,
                                   0x24AB, 0x44AB, 0x64AB, 0x84AB,
                                   0x24B0, 0x44B0, 0x64B0, 0x84B0,
                                   0x24FF, 0x44FF, 0x64FF, 0x84FF,
                                   0x1828, 0x182D, 0x1830
                                  };   // SCROD PS set/status

extern unsigned sizeReadIgnoreRegs;

namespace Belle2 {
  class HSLB;
}

int B2L_FullAddr(const int registerAddr, const int carrier = 0, const int asic = 0);
extern "C" int B2L_ReadReg(const int registerAddr, const int scrod, const int carrier = 0,
                           const int asic = 0, const unsigned read_retry = 3);
int B2L_ReadReg(Belle2::HSLB& hslb, const int registerAddr, const int carrier = 0,
                const int asic = 0, const unsigned read_retry = 3);
extern "C" int B2L_WriteReg(const int registerAddr, const int value, const int scrod, const int carrier = 0,
                            const int asic = 0, const unsigned write_retry = 25);
int B2L_WriteReg(Belle2::HSLB& hslb, const int registerAddr, const int value, const int carrier = 0,
                 const int asic = 0, const unsigned write_retry = 25);
extern "C" int B2L_Reset(const int scrod);

extern "C" float TestPythonAdd(const int a, const int b);

#endif
