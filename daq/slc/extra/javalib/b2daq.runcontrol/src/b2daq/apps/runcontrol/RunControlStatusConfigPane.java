/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.HBox;

/**
 *
 * @author tkonno
 */
class RunControlStatusConfigPane extends HBox {

    @FXML
    private Label labelName;
    @FXML
    private TextField fieldName;
    @FXML
    private TextField fieldFormat;
    @FXML
    private TextField fieldRevision;

    public Label labelNamePropery() {
        return labelName;
    }

    public TextField fieldNamePropery() {
        return fieldName;
    }

    public TextField fieldFormatPropery() {
        return fieldFormat;
    }

    public TextField fieldRevisionPropery() {
        return fieldRevision;
    }

    public RunControlStatusConfigPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("RunControlStatusConfigPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }

    public RunControlStatusConfigPane(String node, String name, String format, int revision) {
        this();
        labelName.setText(node);
        fieldName.setText(name);
        fieldFormat.setText(format);
        fieldRevision.setText(""+revision);
    }
}
