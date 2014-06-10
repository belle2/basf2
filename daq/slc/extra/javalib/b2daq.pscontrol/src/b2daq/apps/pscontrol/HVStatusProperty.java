/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.hvcontrol.core.HVState;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.FloatProperty;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.ObjectProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleFloatProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleObjectProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/**
 *
 * @author tkonno
 */
public class HVStatusProperty {

    private final SimpleIntegerProperty index;
    private final SimpleIntegerProperty crate;
    private final SimpleIntegerProperty slot;
    private final SimpleIntegerProperty channel;
    private final SimpleBooleanProperty turnon;
    private final SimpleObjectProperty state;
    private final SimpleStringProperty valueset;
    private final SimpleFloatProperty voltage;
    private final SimpleFloatProperty current;
    private final SimpleFloatProperty rampupSpeed;
    private final SimpleFloatProperty rampdownSpeed;
    private final SimpleFloatProperty voltageLimit;
    private final SimpleFloatProperty currentLimit;
    private final SimpleFloatProperty voltageDemand;

    public HVStatusProperty() {
        this.index = new SimpleIntegerProperty(0);
        this.crate = new SimpleIntegerProperty(0);
        this.slot = new SimpleIntegerProperty(0);
        this.channel = new SimpleIntegerProperty(0);
        this.turnon = new SimpleBooleanProperty(false);
        this.state = new SimpleObjectProperty(new HVState());
        this.valueset = new SimpleStringProperty("");
        this.voltage = new SimpleFloatProperty(0);
        this.current = new SimpleFloatProperty(0);
        this.rampupSpeed = new SimpleFloatProperty(0);
        this.rampdownSpeed = new SimpleFloatProperty(0);
        this.voltageLimit = new SimpleFloatProperty(0);
        this.currentLimit = new SimpleFloatProperty(0);
        this.voltageDemand = new SimpleFloatProperty(0);
    }

    public HVStatusProperty(int index, int slot, int channel) {
        this.index = new SimpleIntegerProperty(index);
        this.crate = new SimpleIntegerProperty(0);
        this.slot = new SimpleIntegerProperty(slot);
        this.channel = new SimpleIntegerProperty(channel);
        this.turnon = new SimpleBooleanProperty(false);
        this.state = new SimpleObjectProperty(new HVState());
        this.valueset = new SimpleStringProperty("");
        this.voltage = new SimpleFloatProperty(0);
        this.current = new SimpleFloatProperty(0);
        this.rampupSpeed = new SimpleFloatProperty(0);
        this.rampdownSpeed = new SimpleFloatProperty(0);
        this.voltageLimit = new SimpleFloatProperty(0);
        this.currentLimit = new SimpleFloatProperty(0);
        this.voltageDemand = new SimpleFloatProperty(0);
    }

    public HVStatusProperty(int index, int crate, int slot, int channel, 
            boolean turnon, String valueset, 
            float rampupSpeed, float rampdownSpeed,
            float voltageLimit, float currentLimit, float voltageDemand) {
        this.index = new SimpleIntegerProperty(index);
        this.crate = new SimpleIntegerProperty(crate);
        this.slot = new SimpleIntegerProperty(slot);
        this.channel = new SimpleIntegerProperty(channel);
        this.turnon = new SimpleBooleanProperty(turnon);
        this.state = new SimpleObjectProperty(new HVState());
        this.valueset = new SimpleStringProperty(valueset);
        this.voltage = new SimpleFloatProperty(0);
        this.current = new SimpleFloatProperty(0);
        this.rampupSpeed = new SimpleFloatProperty(rampupSpeed);
        this.rampdownSpeed = new SimpleFloatProperty(rampdownSpeed);
        this.voltageLimit = new SimpleFloatProperty(voltageLimit);
        this.currentLimit = new SimpleFloatProperty(currentLimit);
        this.voltageDemand = new SimpleFloatProperty(voltageDemand);
    }

    public int getIndex() {
        return index.get();
    }

    public int getCrate() {
        return crate.get();
    }

    public int getSlot() {
        return slot.get();
    }

    public int getChannel() {
        return channel.get();
    }

    public boolean getTurnon() {
        return turnon.get();
    }

    public float getVoltage() {
        return voltage.get();
    }

    public float getCurrent() {
        return current.get();
    }

    public float getRampupSpeed() {
        return rampupSpeed.get();
    }

    public float getRampdownSpeed() {
        return rampdownSpeed.get();
    }

    public float getVoltageLimit() {
        return voltageLimit.get();
    }

    public float getCurrentLimit() {
        return currentLimit.get();
    }

    public float getVoltageDemand() {
        return voltageDemand.get();
    }

    public String getValuest() {
        return valueset.get();
    }

    public HVState getState() {
        return (HVState)state.get();
    }

    public void setState(HVState state) {
        this.state.set(state);
    }

    public void setIndex(int index) {
        this.index.set(index);
    }

    public void setCrate(int crate) {
        this.crate.set(crate);
    }

    public void setSlot(int slot) {
        this.slot.set(slot);
    }

    public void setChannel(int channel) {
        this.channel.set(channel);
    }

    public void setTurnon(boolean turnon) {
        this.turnon.set(turnon);
    }

    public void setVoltage(float voltage) {
        this.voltage.set(voltage);
    }

    public void setCurrent(float current) {
        this.current.set(current);
    }

    public void setRampupSpeed(float rampup) {
        rampupSpeed.set(rampup);
    }

    public void setRampdownSpeed(float rampdown) {
        rampdownSpeed.set(rampdown);
    }

    public void setVoltageLimit(float voltage) {
        this.voltageLimit.set(voltage);
    }

    public void setCurrentLimit(float current) {
        this.currentLimit.set(current);
    }

    public void setVoltageDemand(float voltageDemand) {
        this.voltageDemand.set(voltageDemand);
    }

    public void setValuest(String valueset) {
        this.valueset.set(valueset);
    }

    public ObjectProperty stateProperty() {
        return state;
    }

    public IntegerProperty indexProperty() {
        return index;
    }

    public IntegerProperty crateProperty() {
        return crate;
    }

    public IntegerProperty slotProperty() {
        return slot;
    }

    public IntegerProperty channelProperty() {
        return channel;
    }

    public BooleanProperty turnonProperty() {
        return turnon;
    }

    public FloatProperty voltageProperty() {
        return voltage;
    }

    public FloatProperty currentProperty() {
        return current;
    }

    public FloatProperty rampdownSpeedProperty() {
        return rampdownSpeed;
    }

    public FloatProperty voltageLimitProperty() {
        return voltageLimit;
    }

    public FloatProperty currentLimitProperty() {
        return currentLimit;
    }

    public FloatProperty voltageDemandProperty() {
        return voltageDemand;
    }

    public StringProperty valuesetProperty() {
        return valueset;
    }

}
