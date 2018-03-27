#ifndef _Belle2_ECLFEE_h
#define _Belle2_ECLFEE_h

#include <daq/slc/copper/FEE.h>
#include <functional>

namespace Belle2 {

  class ECLFEE : public FEE {

  public:
    ECLFEE();
    virtual ~ECLFEE() throw() {}

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  private:
    /********************** AUXILLARY FUNCTIONS **********************/

    /**
     * Name of database entry for the specific base_name ("relay", "comp", "pot", "thr")
     * col, sh, ch, start from 1
     * ch == 0 means this value can't be set for specific channel
     *
     * See method code for full name format.
     */
    std::string getValName(const DBObject& obj, const char* base_name,
                           int col, int sh, int ch = 0);
    /**
     * @return Collector id for this ECLFEE object.
     */
    int getCprCollector(int finid, std::string hostname);
    /**
     * @return Number of shapers in collector
     */
    int getShapersInCollector(int col);

    /********************** SHAPER OPERATIONS **********************/

    /**
     * Write to shaper register.
     * @param sh_mask 12-bit shaper mask
     */
    void rio_sh_wreg(RCCallback& callback, HSLB& hslb, unsigned int sh_mask, unsigned int reg_num, unsigned int reg_wdata);

  };

}

#endif
