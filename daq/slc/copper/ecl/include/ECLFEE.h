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

    /********************** BOOT PROCEDURES **********************/

    /**
     * Loads firmware to shapers
     * (aka loshf, load shaper firmware)
     */
    void loadShFw(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    /**
     * (aka loshc, load shaper coefficients)
     */
    void loadShCoefs(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    /**
     * (aka loshp, load shaper parameters)
     */
    void loadShPars(RCCallback& callback, HSLB& hslb, const DBObject& obj);

    /********************** SHAPER OPERATIONS **********************/

    /**
     * Write to shaper register.
     * @param sh_mask 12-bit shaper mask
     */
    void rio_sh_wreg(RCCallback& callback, HSLB& hslb, unsigned int sh_mask, unsigned int reg_num, unsigned int reg_wdata);

    /**
     * Write to shaper registers using passed function write_reg
     * @param base_name Name of value to be written ("thr", "relay", "comp", "pot").
     *                  Used for lookup in the database.
     * @param b8_cmd    Command to be written in control register 0x00B8
     */
    void writeToShaper(RCCallback& callback, HSLB& hslb, const DBObject& obj,
                       const char* base_name, int b8_cmd,
                       std::function<void (int mask, int vals[16])> write_reg,
                       bool value_per_channel = true);
    /**
     * @param threshold_type
     *        0-hit, 1-low ampl, 2-skip ampl
     */
    void writeThresholds(RCCallback& callback, HSLB& hslb, const DBObject& obj,
                         int threshold_type);
    /**
     * Write potentiometer (aka attenuator) data.
     */
    void writeAttnData(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    /**
     * Write timing adjustments for all shapers (registers 0x20..0x2F)
     */
    void writeADCComp(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    /**
     * Sets value of reg 0x0040 for all shapers.
     */
    void writeRelays(RCCallback& callback, HSLB& hslb, const DBObject& obj);

  };

}

#endif
