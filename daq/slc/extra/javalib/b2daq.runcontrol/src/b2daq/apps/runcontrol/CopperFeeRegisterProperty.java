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
public class CopperFeeRegisterProperty extends DBObjectProperty {

    private final StringProperty copperID = new SimpleStringProperty("cpr000");
    private final StringProperty paramName = new SimpleStringProperty("");
    private final IntegerProperty address = new SimpleIntegerProperty(1);
    private final IntegerProperty size = new SimpleIntegerProperty(1);

    public CopperFeeRegisterProperty(String node, String name, int id) {
        super(node, "copper.fee.register", name, id, new Date());
    }

    public CopperFeeRegisterProperty(String node) {
        this(node, "", 0);
    }

    public CopperFeeRegisterProperty() {
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

    public StringProperty paramNameProperty() {
        return paramName;
    }

    public String getParamName() {
        return paramName.get();
    }

    public void setParamName(String paramName) {
        this.paramName.set(paramName);
    }

    public IntegerProperty addressProperty() {
        return address;
    }

    public int getAddress() {
        return address.get();
    }

    public void setAddress(int id) {
        this.address.set(id);
    }

    public IntegerProperty sizeProperty() {
        return size;
    }

    public int getSize() {
        return size.get();
    }

    public void setSize(int id) {
        this.size.set(id);
    }
    
}
