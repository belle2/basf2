/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.database;

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
public class DBObjectProperty {

    private final SimpleStringProperty node;
    private final SimpleStringProperty table;
    private final SimpleStringProperty name;
    private final SimpleIntegerProperty id;
    private final SimpleObjectProperty date;
    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy");//"yyyy/MM/dd HH:mm:ss"

    public DBObjectProperty() {
        this.node = new SimpleStringProperty("");
        this.table = new SimpleStringProperty("");
        this.name = new SimpleStringProperty("");
        this.id = new SimpleIntegerProperty(0);
        this.date = new SimpleObjectProperty(new Date());
    }

    public DBObjectProperty(String node, String table, String name, int id, Date date) {
        this.node = new SimpleStringProperty(node);
        this.table = new SimpleStringProperty(table);
        this.name = new SimpleStringProperty(name);
        this.id = new SimpleIntegerProperty(id);
        this.date = new SimpleObjectProperty(date);
    }

    public DBObjectProperty(String node, String table, String name, int id) {
        this(node, table, name, id, new Date());
    }

    public DBObjectProperty(String node, String table) {
        this(node, table, "", 0, new Date());
    }

    public String getNode() {
        return node.get();
    }

    public String getTable() {
        return table.get();
    }

    public String getName() {
        return name.get();
    }

    public int getId() {
        return id.get();
    }

    public Date getDate() {
        return (Date) date.get();
    }

    public String getDateText() {
        return dateformat.format(getDate());
    }

    public void setNode(String node) {
        this.node.set(node);
    }

    public void setTable(String table) {
        this.table.set(table);
    }

    public void setId(int id) {
        this.id.set(id);
    }

    public void setDate(Date date) {
        this.date.set(date);
    }

    public StringProperty nodeProperty() {
        return node;
    }

    public StringProperty tableProperty() {
        return table;
    }

    public IntegerProperty idProperty() {
        return id;
    }

    public ObjectProperty dateProperty() {
        return date;
    }
}
