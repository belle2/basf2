/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.database;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 *
 * @author tkonno
 */
public class DBObjectProperty {

    private String node = new String();
    private String table = new String();
    private String name = new String();
    private int index = 0;
    private int id = 0;
    private Date date = new Date();
    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy");//"yyyy/MM/dd HH:mm:ss"

    public DBObjectProperty() {
        this.node = "";
        this.table = "";
        this.name = "";
        this.index = 0;
        this.id = 0;
        this.date = new Date();
    }

    public DBObjectProperty(String node, String table, String name, int id, Date date) {
        this.node = node;
        this.table = table;
        this.name = name;
        this.index = 0;
        this.id = id;
        this.date = date;
    }

    public DBObjectProperty(String node, String table, String name, int id) {
        this(node, table, name, id, new Date());
    }

    public DBObjectProperty(String node, String table) {
        this(node, table, "", 0, new Date());
    }

    public String getNode() {
        return node;
    }

    public String getTable() {
        return table;
    }

    public String getName() {
        return name;
    }

    public int getIndex() {
        return index;
    }

    public int getId() {
        return id;
    }

    public Date getDate() {
        return date;
    }

    public String getDateText() {
        return dateformat.format(getDate());
    }

    public void setNode(String node) {
        this.node = node;
    }

    public void setTable(String table) {
        this.table = table;
    }

    public void setIndex(int id) {
        this.index = id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public void setDate(Date date) {
        this.date = date;
    }

    public String nodeProperty() {
        return node;
    }

    public String tableProperty() {
        return table;
    }

    public int indexProperty() {
        return index;
    }

    public int idProperty() {
        return id;
    }

    public Date dateProperty() {
        return date;
    }
}
