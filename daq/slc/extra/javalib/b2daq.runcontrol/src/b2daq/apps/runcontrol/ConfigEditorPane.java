/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.nsm.NSMListenerService;
import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;

/**
 *
 * @author tkonno
 */
final class ConfigEditorPane extends VBox {

    @FXML
    private ComboBox comboConfigName;
    @FXML
    private Button buttonCreateNew;
    @FXML
    private Label labelConfigId;
    @FXML
    private Label labelDBTable;
    @FXML
    private VBox boxConfig;

    private String nodename = ""; 
    private ConfigManager manager;
    
    public ComboBox comboConfigNamePropery() {
        return comboConfigName;
    }

    public Button buttonCreateNewPropery() {
        return buttonCreateNew;
    }

    public Label labelConfigIdPropery() {
        return labelConfigId;
    }

    public Label labelDBTablePropery() {
        return labelDBTable;
    }

    public VBox boxConfigPropery() {
        return boxConfig;
    }

    public ConfigEditorPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("ConfigEditorPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        VBox.setVgrow(this, Priority.ALWAYS);
        comboConfigName.setOnAction((event) -> {
            String confname = (String)comboConfigName.getSelectionModel().getSelectedItem();
            if (nodename.length() > 0 && confname.length() > 0 && labelDBTable.getText().length() > 0)
                NSMListenerService.requestDBGet(nodename, labelDBTable.getText(), confname, 0);
        });
        buttonCreateNew.setOnAction((event) -> {
            String confname = (String)comboConfigName.getSelectionModel().getSelectedItem()+"_1";
            String newconf = NewConfigDialog.showDialog(null, "New configuration", nodename, confname);
            ConfigObject obj = manager.update(nodename, newconf);
            NSMListenerService.requestDBSet(obj);
            //obj.print();
            System.out.println(newconf);
        });
    }

    public ConfigEditorPane(String name, Node node, ConfigManager man) {
        this();
        setPane(name, node, man);
    }
    
    public void setPane(String name, Node node, ConfigManager man) {
        boxConfig.getChildren().add(node);
        VBox.setVgrow(node, Priority.ALWAYS);
        nodename = name;
        manager = man;
    }
}
