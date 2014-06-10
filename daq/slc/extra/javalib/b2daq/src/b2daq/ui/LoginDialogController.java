/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.ui;

import b2daq.nsm.NSMConfig;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
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
public class LoginDialogController implements Initializable {

    @FXML
    private Label label;
    @FXML
    private TextField hostname;
    @FXML
    private TextField port;
    @FXML
    private TextField nsmnode;
    @FXML
    private TextField nsmhost;
    @FXML
    private TextField nsmport;
    @FXML
    private Button loginButton;
    @FXML
    private Button cancelButton;
    @FXML
    private NSMConfig config;// = new NSMConfig();
//    <fx:define> 
//        <NSMConfig fx:id="config" hostname="b2slow2.kek.jp" />
//    </fx:define>

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
        config.hostnameProperty().bind(hostname.textProperty());
        port.textProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable, Object oldValue, Object newValue) {
                try {
                    config.portProperty().set(Integer.parseInt(port.getText()));
                } catch (Exception e) {}
            }
        });
        config.nsmNodeProperty().bind(nsmnode.textProperty());
        config.nsmHostProperty().bind(nsmhost.textProperty());
        nsmport.textProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable, Object oldValue, Object newValue) {
                try {
                    config.nsmPortProperty().set(Integer.parseInt(nsmport.getText()));
                } catch (Exception e) {}
            }
        });
        loginButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
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

    public void set(String hostname, int port, 
            String nsmhost, String nsmnode, int nsmport) {
        this.hostname.setText(hostname);
        this.port.setText(""+port);
        this.nsmhost.setText(nsmhost);
        this.nsmnode.setText(nsmnode);
        this.nsmport.setText(""+nsmport);
    }

    public NSMConfig getNSMConfig() {
        return isOK?config:null;
    }
}
