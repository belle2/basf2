/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import javafx.beans.property.SimpleStringProperty;

/**
 *
 * @author tkonno
 */
public class ConfigParameter {

    private SimpleStringProperty m_name;
    private SimpleStringProperty m_value;

    public SimpleStringProperty nameProperty() {
        if (m_name == null) {
            m_name = new SimpleStringProperty(this, "name");
        }
        return m_name;
    }

    public SimpleStringProperty valueProperty() {
        if (m_value == null) {
            m_value = new SimpleStringProperty(this, "value");
        }
        return m_value;
    }

    ConfigParameter(String name, String value) {
        m_name = new SimpleStringProperty(name);
        m_value = new SimpleStringProperty(value);
    }

    public String getName() {
        return m_name.get();
    }

    public void setName(String fName) {
        m_name.set(fName);
    }

    public String getValue() {
        return m_value.get();
    }

    public void setValue(String fName) {
        m_value.set(fName);
    }

}
