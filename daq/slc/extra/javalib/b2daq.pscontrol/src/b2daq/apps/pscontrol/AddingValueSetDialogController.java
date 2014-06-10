/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import java.net.URL;
import java.util.ResourceBundle;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class AddingValueSetDialogController implements Initializable {

    @FXML
    private Label label;
    @FXML
    private TextField input;
    @FXML
    private Button okButton;
    @FXML
    private Button cancelButton;

    private boolean isOK = false;

    @FXML
    private void handleOKButton() {
        handleCloseAction(true);
    }

    @FXML
    private void handleCancelButton() {
        handleCloseAction(false);
    }

    private void handleCloseAction(boolean isOK) {
        this.isOK = isOK;
        label.getScene().getWindow().hide();
    }

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        okButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(true);
                }
            }
        });
        cancelButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(false);
                }
            }
        });
    }

    public void set(String message, String prompt) {
        label.setText(message);
        input.setText(prompt);
    }

    public String getText() {
        return (isOK) ? input.getText() : null;
    }
}
