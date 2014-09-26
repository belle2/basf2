/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm.ui;

import java.text.SimpleDateFormat;
import java.util.Date;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.ObjectProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleObjectProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

/**
 *
 * @author tkonno
 */
public class NSMDataProperty {

    private final SimpleStringProperty dataname;
    private final SimpleStringProperty format;
    private final SimpleIntegerProperty revision;
    private final SimpleObjectProperty date;
    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy");//"yyyy/MM/dd HH:mm:ss"

    public NSMDataProperty() {
        this.dataname = new SimpleStringProperty("");
        this.format = new SimpleStringProperty("");
        this.date = new SimpleObjectProperty(new Date());
        this.revision = new SimpleIntegerProperty(0);
    }

    public NSMDataProperty(String dataname, String format, int revision, Date date) {
        this.dataname = new SimpleStringProperty(dataname);
        this.format = new SimpleStringProperty(format);
        this.date = new SimpleObjectProperty(date);
        this.revision = new SimpleIntegerProperty(revision);
    }

    public NSMDataProperty(String dataname, String format, int revision) {
        this(dataname, format, revision, new Date());
    }

    public String getDataname() {
        return dataname.get();
    }

    public String getFormat() {
        return format.get();
    }

    public int getRevision() {
        return revision.get();
    }

    public Date getDate() {
        return (Date) date.get();
    }

    public String getDateText() {
        return dateformat.format(getDate());
    }

    public void setDataname(String dataname) {
        this.dataname.set(dataname);
    }

    public void setFormat(String format) {
        this.format.set(format);
    }

    public void setRevision(int revision) {
        this.revision.set(revision);
    }

    public void setDate(Date date) {
        this.date.set(date);
    }

    public StringProperty datanameProperty() {
        return dataname;
    }

    public StringProperty formatProperty() {
        return format;
    }

    public IntegerProperty revisionProperty() {
        return revision;
    }

    public ObjectProperty dateProperty() {
        return date;
    }
}
