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
public class CopperFeeParameterProperty extends DBObjectProperty {

    private final StringProperty copperID = new SimpleStringProperty("cpr000");
    private final StringProperty board = new SimpleStringProperty("");
    private final IntegerProperty slot = new SimpleIntegerProperty(0);
    private final StringProperty paramName = new SimpleStringProperty("");
    private final IntegerProperty arrayIndex = new SimpleIntegerProperty(1);
    private final IntegerProperty value = new SimpleIntegerProperty(1);
    private final IntegerProperty address = new SimpleIntegerProperty(1);

    public CopperFeeParameterProperty(String node, String name, int id) {
        super(node, "copper.fee.parameter", name, id, new Date());
    }

    public CopperFeeParameterProperty(String node) {
        this(node, "", 0);
    }

    public CopperFeeParameterProperty() {
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

    public IntegerProperty arrayIndexProperty() {
        return arrayIndex;
    }

    public int getArrayIndex() {
        return arrayIndex.get();
    }

    public void setArrayIndex(int id) {
        this.arrayIndex.set(id);
    }

    public IntegerProperty valueProperty() {
        return value;
    }

    public int getValue() {
        return value.get();
    }

    public void setValue(int id) {
        this.value.set(id);
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

}
