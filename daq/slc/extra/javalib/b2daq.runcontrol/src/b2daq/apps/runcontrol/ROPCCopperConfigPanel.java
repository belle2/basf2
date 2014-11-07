/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.CheckBox;
import javafx.scene.control.TextField;
import javafx.scene.layout.HBox;

/**
 *
 * @author tkonno
 */
class ROPCCopperConfigPanel extends HBox {

    @FXML
    private CheckBox checkUsed;
    @FXML
    private TextField fieldHostName;
    @FXML
    private TextField fieldPort;

    public CheckBox checkUsedPropery() {
        return checkUsed;
    }

    public TextField fieldHostNamePropery() {
        return fieldHostName;
    }

    public TextField fieldPortPropery() {
        return fieldPort;
    }

    public ROPCCopperConfigPanel() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("ROPCCopperConfigPanel.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        fieldHostName.disableProperty().bind(checkUsed.selectedProperty().not());
        fieldPort.disableProperty().bind(checkUsed.selectedProperty().not());
    }

    public ROPCCopperConfigPanel(String hostname, int port, boolean used) {
        this();
        fieldHostName.setText(hostname);
        fieldPort.setText(""+port);
        checkUsed.setSelected(used);
    }
}
