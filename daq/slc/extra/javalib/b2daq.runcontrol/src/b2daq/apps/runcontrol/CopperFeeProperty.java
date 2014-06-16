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
public class CopperFeeProperty extends DBObjectProperty {

    private final StringProperty copperID = new SimpleStringProperty("cpr000");
    private final StringProperty board = new SimpleStringProperty("");
    private final IntegerProperty slot = new SimpleIntegerProperty(0);
    private final StringProperty firmware = new SimpleStringProperty("");
    private final BooleanProperty used = new SimpleBooleanProperty(true);
    private final ObservableList<CopperFeeRegisterProperty> register = 
            FXCollections.observableArrayList();
    private final ObservableList<CopperFeeParameterProperty> parameter = 
            FXCollections.observableArrayList();

    public CopperFeeProperty(String node, String name, int id) {
        super(node, "copper.fee", name, id, new Date());
    }

    public CopperFeeProperty(String node) {
        this(node, "", 0);
    }

    public CopperFeeProperty() {
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
    
    public StringProperty boardProperty() {
        return board;
    }

    public String getBoard() {
        return board.get();
    }

    public void setBoard(String board) {
        this.board.set(board);
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

    public StringProperty firmwareProperty() {
        return firmware;
    }

    public String getFirmware() {
        return firmware.get();
    }

    public void setFirmware(String id) {
        this.firmware.set(id);
    }

    public BooleanProperty usedProperty() {
        return used;
    }

    public Boolean getUsed() {
        return used.get();
    }

    public void setUsed(Boolean used) {
        this.used.set(used);
    }

    public ObservableList<CopperFeeRegisterProperty> getRegister() {
        return register;
    }
    
    public void setRegister(ObservableList<CopperFeeRegisterProperty> register) {
        this.register.setAll(register);
    }

    public void setParameter(ObservableList<CopperFeeParameterProperty> paramter) {
        this.parameter.setAll(paramter);
    }

    public ObservableList<CopperFeeParameterProperty> getParameter() {
        return parameter;
    }
}
