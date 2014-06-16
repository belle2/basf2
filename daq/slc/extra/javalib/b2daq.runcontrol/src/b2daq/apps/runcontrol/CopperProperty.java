/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.DBObjectProperty;
import java.util.Date;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;

/**
 *
 * @author tkonno
 */
public class CopperProperty extends DBObjectProperty {

    private final StringProperty copperID = new SimpleStringProperty("cpr000");
    private final BooleanProperty hslbA = new SimpleBooleanProperty(false);
    private final BooleanProperty hslbB = new SimpleBooleanProperty(false);
    private final BooleanProperty hslbC = new SimpleBooleanProperty(false);
    private final BooleanProperty hslbD = new SimpleBooleanProperty(false);
    private final StringProperty basf2script = new SimpleStringProperty("");
    private final IntegerProperty dmaLimit = new SimpleIntegerProperty(100);
    private final StringProperty eb0Host = new SimpleStringProperty("ropc01");
    private final IntegerProperty eb0Port = new SimpleIntegerProperty(33000);
    private final CopperSetupProperty setup;
    private final ObservableList<CopperFeeProperty> fee
            = FXCollections.observableArrayList();

    public CopperProperty(String node, String name, int id) {
        super(node, "copper", name, id, new Date());
        setup = new CopperSetupProperty(node, name, id);
    }

    public CopperProperty(String node) {
        this(node, "", 0);
    }

    public CopperProperty() {
        this("", "", 0);
    }

    public StringProperty copperIDProperty() {
        return copperID;
    }

    public String getCopperID() {
        return copperID.get();
    }

    public void setCopperID(String script) {
        copperID.set(script);
    }

    public BooleanProperty hslbAProperty() {
        return hslbA;
    }

    public boolean getHslbA() {
        return hslbA.get();
    }

    public void setHslbA(boolean hslb) {
        hslbA.set(hslb);
    }

    public BooleanProperty hslbBProperty() {
        return hslbB;
    }

    public boolean getHslbB() {
        return hslbB.get();
    }

    public void setHslbB(boolean hslb) {
        hslbB.set(hslb);
    }

    public BooleanProperty hslbCProperty() {
        return hslbC;
    }

    public boolean getHslbC() {
        return hslbC.get();
    }

    public void setHslbC(boolean hslb) {
        hslbC.set(hslb);
    }

    public BooleanProperty hslbDProperty() {
        return hslbD;
    }

    public boolean getHslbD() {
        return hslbD.get();
    }

    public void setHslbD(boolean hslb) {
        hslbD.set(hslb);
    }

    public StringProperty basf2scriptProperty() {
        return basf2script;
    }

    public String getBasf2script() {
        return basf2script.get();
    }

    public void setBasf2script(String script) {
        basf2script.set(script);
    }

    public IntegerProperty dmaLimitProperty() {
        return dmaLimit;
    }

    public int getDmaLimit() {
        return dmaLimit.get();
    }

    public void setDmaLimit(int limit) {
        dmaLimit.set(limit);
    }

    public StringProperty eb0HostProperty() {
        return eb0Host;
    }

    public String getEb0Host() {
        return eb0Host.get();
    }

    public void setEb0Host(String script) {
        eb0Host.set(script);
    }

    public IntegerProperty eb0PortProperty() {
        return eb0Port;
    }

    public int getEb0Port() {
        return eb0Port.get();
    }

    public void setEb0Port(int limit) {
        eb0Port.set(limit);
    }

    public CopperSetupProperty getSetup() {
        return setup;
    }

    public ObservableList<CopperFeeProperty> getFee() {
        return fee;
    }
}
