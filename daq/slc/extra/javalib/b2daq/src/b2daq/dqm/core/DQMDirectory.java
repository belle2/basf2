/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqm.core;

import b2daq.core.Reader;
import b2daq.core.Serializable;
import b2daq.core.Writer;
import java.io.IOException;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;

/**
 *
 * @author tkonno
 */
public class DQMDirectory implements Serializable {

    private final StringProperty name = new SimpleStringProperty();
    private final ObservableList<DQMDirectory> directories = FXCollections.observableArrayList();
    private final ObservableList<Histo> histograms = FXCollections.observableArrayList();

    public DQMDirectory() {
    
    }
    
    public DQMDirectory(String name) {
        this.name.set(name);
    }
    
    public String getName() {
        return name.get();
    }
    
    public void setName(String name) {
        this.name.set(name);
    }
    
    public StringProperty nameProperty() {
        return name;
    }
    
    public ObservableList<DQMDirectory> getDirectories() {
        return directories;
    }

    public ObservableList<Histo> getHistograms() {
        return histograms;
    }

    public void setDirectories(ObservableList<DQMDirectory> directories) {
        this.directories.setAll(directories);
    }

    public void setHistograms(ObservableList<Histo> histograms) {
        this.histograms.setAll(histograms);
    }

    @Override
    public void readObject(Reader reader) throws IOException {
        this.name.set(reader.readString());
        directories.clear();
        int ndir = reader.readInt();
        for (int n = 0; n < ndir; n++) {
            DQMDirectory dir = new DQMDirectory();
            reader.readObject(dir);
            directories.add(dir);
        }
        histograms.clear();
        int nhist = reader.readInt();
        for (int n = 0; n < nhist; n++) {
            Histo hist = HistoFactory.create(reader.readString());
            if (hist != null) {
                hist.readConfig(reader);
                histograms.add(hist);
            }
        }
        
    }

    @Override
    public void writeObject(Writer writer) throws IOException {
        writer.writeString(name.get());
        writer.writeInt(directories.size());
        for (DQMDirectory dir : directories) {
            writer.writeObject(dir);
        }
        writer.writeInt(histograms.size());
        for (Histo hist : histograms) {
            hist.writeConfig(writer);
        }
    }

}
