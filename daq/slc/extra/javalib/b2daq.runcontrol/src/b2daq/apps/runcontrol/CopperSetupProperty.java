/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package b2daq.apps.runcontrol;

import b2daq.database.DBObjectProperty;
import java.util.Date;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/**
 *
 * @author tkonno
 */
public class CopperSetupProperty extends DBObjectProperty {
    
    private final IntegerProperty crate = new SimpleIntegerProperty(0);
    private final IntegerProperty slot = new SimpleIntegerProperty(0);
    private final StringProperty copperID = new SimpleStringProperty("cpr000");
    private final StringProperty hostname = new SimpleStringProperty("cpr000");
    private final StringProperty serialID = new SimpleStringProperty();
    private final StringProperty prpmc = new SimpleStringProperty("");
    private final StringProperty ttrx = new SimpleStringProperty("");
    private final StringProperty ttrxFirmware = new SimpleStringProperty("");
    private final StringProperty hslbA = new SimpleStringProperty("");
    private final StringProperty hslbB = new SimpleStringProperty("");
    private final StringProperty hslbC = new SimpleStringProperty("");
    private final StringProperty hslbD = new SimpleStringProperty("");
    private final StringProperty hslbFirmware = new SimpleStringProperty("");
    private final StringProperty basf2script = new SimpleStringProperty("");

    public CopperSetupProperty(String node, String name, int id) {
        super(node, "copper.setup", name, id, new Date());
    }

    public CopperSetupProperty(String node) {
        this(node, "", 0);
    }

    public CopperSetupProperty() {
        this("", "", 0);
    }
    
    public StringProperty copperIDProperty() {
        return copperID;
    }

    public String getCopperID() {
        return copperID.get();
    }

    public void setCopperID(String copperID) {
        this.copperID.set(copperID);
    }

    public StringProperty hostnameProperty() {
        return hostname;
    }

    public String getHostname() {
        return hostname.get();
    }

    public void setHostname(String hostname) {
        this.hostname.set(hostname);
    }

    public StringProperty serialIDProperty() {
        return serialID;
    }

    public String getSerialID() {
        return serialID.get();
    }

    public void setSerialID(String id) {
        this.serialID.set(id);
    }

    public StringProperty prpmcProperty() {
        return prpmc;
    }

    public String getPrpmc() {
        return prpmc.get();
    }

    public void setPrpmc(String prpmc) {
        this.prpmc.set(prpmc);
    }

    public StringProperty ttrxProperty() {
        return ttrx;
    }

    public String getTtrx() {
        return ttrx.get();
    }

    public void setTtrx(String ttrx) {
        this.ttrx.set(ttrx);
    }

    public StringProperty ttrxFirmwareProperty() {
        return ttrxFirmware;
    }

    public String getTtrxFirmware() {
        return ttrxFirmware.get();
    }

    public void setTtrxFirmware(String ttrx) {
        this.ttrxFirmware.set(ttrx);
    }

    public StringProperty hslbAProperty() {
        return hslbA;
    }

    public String getHslbA() {
        return hslbA.get();
    }

    public void setHslbA(String hslb) {
        this.hslbA.set(hslb);
    }

    public StringProperty hslbBProperty() {
        return hslbB;
    }

    public String getHslbB() {
        return hslbB.get();
    }

    public void setHslbB(String hslb) {
        this.hslbB.set(hslb);
    }

    public StringProperty hslbCProperty() {
        return hslbC;
    }

    public String getHslbC() {
        return hslbC.get();
    }

    public void setHslbC(String hslb) {
        this.hslbC.set(hslb);
    }

    public StringProperty hslbDProperty() {
        return hslbD;
    }

    public String getHslbD() {
        return hslbD.get();
    }

    public void setHslbD(String hslb) {
        this.hslbD.set(hslb);
    }

    public StringProperty hslbFirmwareProperty() {
        return hslbFirmware;
    }

    public String getHslbFirmware() {
        return hslbFirmware.get();
    }

    public void setHslbFirmware(String hslb) {
        this.hslbFirmware.set(hslb);
    }

    public StringProperty basf2scriptProperty() {
        return basf2script;
    }

    public String getBasf2script() {
        return basf2script.get();
    }

    public void setBasf2script(String script) {
        this.basf2script.set(script);
    }

    public IntegerProperty crateProperty() {
        return crate;
    }

    public int getCrate() {
        return crate.get();
    }

    public void setCrate(int limit) {
        this.crate.set(limit);
    }

    public IntegerProperty slotProperty() {
        return slot;
    }

    public int getSlot() {
        return slot.get();
    }

    public void setSlot(int limit) {
        this.slot.set(limit);
    }

}
