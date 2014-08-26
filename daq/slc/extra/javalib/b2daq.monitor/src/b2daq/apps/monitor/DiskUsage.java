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
public class DiskUsage {

    private final SimpleStringProperty label = new SimpleStringProperty("");
    private final SimpleStringProperty status = new SimpleStringProperty("FINE");
    private final SimpleIntegerProperty nfiles = new SimpleIntegerProperty(0);
    private final SimpleFloatProperty available = new SimpleFloatProperty(0);
    private final SimpleFloatProperty size = new SimpleFloatProperty(0);

    public DiskUsage() {}
    
    public DiskUsage(String label) {
        setLabel(label);
    }

    public String getLabel() {
        return label.get();
    }

    public String getState() {
        return status.get();
    }

    public int getNfiles() {
        return nfiles.get();
    }
    
    public float getAvailable() {
        return available.get();
    }

    public float getSize() {
        return size.get();
    }

    public void setLabel(String label) {
        this.label.set(label);
    }

    public void setStatus(String status) {
        this.status.set(status);
    }

    public void setNfiles(int nfiles) {
        this.nfiles.set(nfiles);
    }

    public void setAvailable(float available) {
        this.available.set(available);
    }

    public void setSize(float size) {
        this.size.set(size);
    }

    public StringProperty labelProperty() {
        return label;
    }

    public StringProperty statusProperty() {
        return status;
    }

    public IntegerProperty nfilesProperty() {
        return nfiles;
    }

    public FloatProperty availableProperty() {
        return available;
    }

    public FloatProperty sizeProperty() {
        return size;
    }

}
