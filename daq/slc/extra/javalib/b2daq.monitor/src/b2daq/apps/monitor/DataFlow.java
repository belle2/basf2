/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.monitor;

import javafx.beans.property.FloatProperty;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleFloatProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/**
 *
 * @author tkonno
 */
public class DataFlow {

    private final SimpleStringProperty node = new SimpleStringProperty("");
    private final SimpleIntegerProperty state = new SimpleIntegerProperty(0);
    private final SimpleFloatProperty loadavg = new SimpleFloatProperty(0);
    private final SimpleIntegerProperty connection_in = new SimpleIntegerProperty(0);
    private final SimpleIntegerProperty nqueue_in = new SimpleIntegerProperty(0);
    private final SimpleIntegerProperty nevent_in = new SimpleIntegerProperty(0);
    private final SimpleFloatProperty evtrate_in = new SimpleFloatProperty(0);
    private final SimpleFloatProperty flowrate_in = new SimpleFloatProperty(0);
    private final SimpleFloatProperty evtsize_in = new SimpleFloatProperty(0);
    private final SimpleIntegerProperty connection_out = new SimpleIntegerProperty(0);
    private final SimpleIntegerProperty nqueue_out = new SimpleIntegerProperty(0);
    private final SimpleIntegerProperty nevent_out = new SimpleIntegerProperty(0);
    private final SimpleFloatProperty evtrate_out = new SimpleFloatProperty(0);
    private final SimpleFloatProperty flowrate_out = new SimpleFloatProperty(0);
    private final SimpleFloatProperty evtsize_out = new SimpleFloatProperty(0);

    public DataFlow() {}

    public String getNode() {
        return node.get();
    }

    public int getState() {
        return state.get();
    }

    public float getLoadavg() {
        return loadavg.get();
    }

    public int getConnectionIn() {
        return connection_in.get();
    }

    public int getQueueIn() {
        return nqueue_in.get();
    }

    public int getNeventIn() {
        return nevent_in.get();
    }

    public float getFreqIn() {
        return evtrate_in.get();
    }

    public float getFlowIn() {
        return flowrate_in.get();
    }

    public float getSizeIn() {
        return evtsize_in.get();
    }

    public int getConnectionOut() {
        return connection_out.get();
    }

    public int getQueueOut() {
        return nqueue_out.get();
    }

    public int getNeventOut() {
        return nevent_out.get();
    }

    public float getEvtrateOut() {
        return evtrate_out.get();
    }

    public float getFlowrateOut() {
        return flowrate_out.get();
    }

    public float getEvtsizeOut() {
        return evtsize_out.get();
    }

    public void setNode(String node) {
        this.node.set(node);
    }

    public void setState(int state) {
        this.state.set(state);
    }

    public void setLoadavg(float loadavg) {
        this.loadavg.set(loadavg);
    }

    public void setConnectionIn(int connection) {
        this.connection_in.set(connection);
    }

    public void setNqueueIn(int queue) {
        this.nqueue_in.set(queue);
    }

    public void setNeventIn(int nevent) {
        this.nevent_in.set(nevent);
    }

    public void setEvtrateIn(float freq) {
        this.evtrate_in.set(freq);
    }

    public void setFlowrateIn(float rate) {
        this.flowrate_in.set(rate);
    }

    public void setEvtsizeIn(float size) {
        this.evtsize_in.set(size);
    }

    public void setConnectionOut(int connection) {
        this.connection_out.set(connection);
    }

    public void setNqueueOut(int queue) {
        this.nqueue_out.set(queue);
    }

    public void setNeventOut(int nevent) {
        this.nevent_out.set(nevent);
    }

    public void setEvtrateOut(float freq) {
        this.evtrate_out.set(freq);
    }

    public void setFlowrateOut(float rate) {
        this.flowrate_out.set(rate);
    }

    public void setEvtsizeOut(float size) {
        this.evtsize_out.set(size);
    }

    public StringProperty nodeProperty() {
        return node;
    }

    public IntegerProperty stateProperty() {
        return state;
    }

    public FloatProperty loadavgProperty() {
        return loadavg;
    }

    public IntegerProperty connectionInProperty() {
        return connection_in;
    }

    public IntegerProperty nqueueInProperty() {
        return nqueue_in;
    }

    public IntegerProperty neventInProperty() {
        return nevent_in;
    }

    public FloatProperty flowrateInProperty() {
        return flowrate_in;
    }

    public FloatProperty evtsizeInProperty() {
        return evtsize_in;
    }

    public FloatProperty evtrateInProperty() {
        return evtrate_in;
    }

    public IntegerProperty connectionOutProperty() {
        return connection_out;
    }

    public IntegerProperty nqueueOutProperty() {
        return nqueue_out;
    }

    public IntegerProperty neventOutProperty() {
        return nevent_out;
    }

    public FloatProperty flowrateOutProperty() {
        return flowrate_out;
    }

    public FloatProperty evtsizeOutProperty() {
        return evtsize_out;
    }

    public FloatProperty evtrateOutProperty() {
        return evtrate_out;
    }

}
