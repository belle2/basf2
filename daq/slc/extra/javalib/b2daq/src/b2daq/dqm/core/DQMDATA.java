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
import java.util.HashMap;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/**
 *
 * @author tkonno
 */
public class DQMDATA implements Serializable {

    private final StringProperty name = new SimpleStringProperty();
    private final IntegerProperty updateid = new SimpleIntegerProperty(-1);
    private final HashMap<String, Histo> histograms = new HashMap<>();

    public DQMDATA() {
    
    }
    
    public DQMDATA(String name) {
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
    
    public int getUpdateid() {
        return updateid.get();
    }
    
    public void setUpdateid(int updateid) {
        this.updateid.set(updateid);
    }
    
    public IntegerProperty updateidProperty() {
        return updateid;
    }
    
    public HashMap<String, Histo> getHistograms() {
        return histograms;
    }

    @Override
    public void readObject(Reader reader) throws IOException {
        this.name.set(reader.readString());
        histograms.clear();
        int nhist = reader.readInt();
        for (int n = 0; n < nhist; n++) {
            String oname = reader.readString();
            String datatype = reader.readString();
            Histo hist = null;
            if (histograms.containsKey(oname)) {
                hist = histograms.get(oname);
                if (!hist.getDataType().matches(datatype)) {
                    hist = null;
                }
            }
            if (hist == null) {
                hist = HistoFactory.create(datatype);
                histograms.put(oname, hist);
            }
            hist.readObject(reader);
        }
    }

    @Override
    public void writeObject(Writer writer) throws IOException {
        writer.writeString(name.get());
        writer.writeInt(histograms.size());
        for (String oname : histograms.keySet()) {
            Histo hist = histograms.get(oname);
            writer.writeString(oname);
            writer.writeObject(hist);
        }
    }

}
