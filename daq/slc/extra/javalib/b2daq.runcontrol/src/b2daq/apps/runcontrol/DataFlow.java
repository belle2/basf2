/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import javafx.beans.property.FloatProperty;
import javafx.beans.property.LongProperty;
import javafx.beans.property.SimpleFloatProperty;
import javafx.beans.property.SimpleLongProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/**
 *
 * @author tkonno
 */
public class DataFlow {

    private final SimpleStringProperty iotype = new SimpleStringProperty("");
    private final SimpleLongProperty count = new SimpleLongProperty(0);
    private final SimpleFloatProperty freq = new SimpleFloatProperty(0);
    private final SimpleFloatProperty rate = new SimpleFloatProperty(0);
    private final SimpleFloatProperty size = new SimpleFloatProperty(0);

    public DataFlow() {}

    public DataFlow(String iotype) {
        this.iotype.set(iotype);
    }
    
    public String getIotype() {
        return iotype.get();
    }

    public long getCount() {
        return (long)count.get();
    }

    public float getRate() {
        return rate.get();
    }

    public float getSize() {
        return size.get();
    }

    public void setIotype(String iotype) {
        this.iotype.set(iotype);
    }

    public void setCount(long count) {
        this.count.set(count);
    }

    public void setRate(float rate) {
        this.rate.set(rate);
    }

    public float getFreq() {
        return freq.get();
    }

    public void setSize(float size) {
        this.size.set(size);
    }

    public void setFreq(float freq) {
        this.freq.set(freq);
    }

    public StringProperty iotypeProperty() {
        return iotype;
    }

    public LongProperty countProperty() {
        return count;
    }

    public FloatProperty rateProperty() {
        return rate;
    }

    public FloatProperty sizeProperty() {
        return size;
    }

    public FloatProperty freqProperty() {
        return freq;
    }

}
