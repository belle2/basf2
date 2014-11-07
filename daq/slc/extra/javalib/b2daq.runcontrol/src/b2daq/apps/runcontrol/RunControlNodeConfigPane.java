/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.Button;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;

/**
 *
 * @author tkonno
 */
class RunControlNodeConfigPane extends HBox {

    @FXML
    private Label labelNodeName;
    @FXML
    private CheckBox checkUsed;
    @FXML
    private CheckBox checkSequential;
    @FXML
    private ComboBox comboConfig;
    @FXML
    private Button buttonNew;

    public Label labelNodeNamePropery() {
        return labelNodeName;
    }

    public CheckBox checkUsedPropery() {
        return checkUsed;
    }

    public CheckBox checkSequentialPropery() {
        return checkSequential;
    }

    public ComboBox comboConfigPropery() {
        return comboConfig;
    }

    public Button buttonNewPropery() {
        return buttonNew;
    }

    public RunControlNodeConfigPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("RunControlNodeConfigPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        checkSequential.disableProperty().bind(checkUsed.selectedProperty().not());
        comboConfig.disableProperty().bind(checkUsed.selectedProperty().not());
        buttonNew.disableProperty().bind(checkUsed.selectedProperty().not());
    }

    public RunControlNodeConfigPane(String name, boolean used, boolean sequential, String configname) {
        this();
        labelNodeName.setText(name);
        checkUsed.setSelected(used);
        checkSequential.setSelected(sequential);
        comboConfig.getItems().add(configname);
        comboConfig.getSelectionModel().selectFirst();
    }
}
