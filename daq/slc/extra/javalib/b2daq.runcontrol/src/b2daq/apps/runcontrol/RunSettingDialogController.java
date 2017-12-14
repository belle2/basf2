/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMVar;
import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class RunSettingDialogController implements Initializable {

    @FXML
    private Label label;
    @FXML
    private TextField expno;
    @FXML
    private TextField runno;
    @FXML
    private TextField operator1;
    @FXML
    private TextField operator2;
    @FXML
    private TextArea comment;
    @FXML
    private Button startButton;
    @FXML
    private Button cancelButton;

    private RunSetting setting = new RunSetting();

    private boolean isStarted = false;
    private String m_nodename;

    @FXML
    private void handleStartButton() {
        handleCloseAction(true);
    }

    @FXML
    private void handleCancelButton() {
        handleCloseAction(false);
    }

    private void handleCloseAction(boolean isStarted) {
        this.isStarted = isStarted;
        label.getScene().getWindow().hide();
        if (this.isStarted) {
            try {
                NSMCommunicator.get().requestVSet(m_nodename, new NSMVar("expno", Integer.parseInt(expno.getText())));
                NSMCommunicator.get().requestVSet(m_nodename, new NSMVar("operators", operator1.getText()+":"+operator2.getText()));
                NSMCommunicator.get().requestVSet(m_nodename, new NSMVar("comment", comment.getText().replace("\n", "<br/>")));
            } catch (IOException ex) {
                Logger.getLogger(RunSettingDialogController.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    @Override
    public void initialize(URL url, ResourceBundle rb) {
        expno.textProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable, Object oldValue, Object newValue) {
                try {
                    setting.expnoProperty().set(Integer.parseInt(expno.getText()));
                } catch (Exception e) {
                }
            }
        });
        runno.textProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable, Object oldValue, Object newValue) {
                try {
                    setting.runnoProperty().set(Integer.parseInt(runno.getText()));
                } catch (Exception e) {
                }
            }
        });
        setting.operator1Property().bind(operator1.textProperty());
        setting.operator2Property().bind(operator2.textProperty());
        setting.commentProperty().bind(comment.textProperty());
        startButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
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

    public void set(String nodename, int expno, int runno, String operators, String comment) {
        m_nodename = nodename.toUpperCase();
        this.expno.setText("" + expno);
        this.runno.setText("" + runno);
        this.comment.setText(comment);
        String[] operator = operators.split(":");
        if (operator.length > 0) {
            this.operator1.setText(operator[0]);
        }
        if (operator.length > 1) {
            this.operator2.setText(operator[1]);
        }
    }

    public RunSetting getSetting() {
        return isStarted ? setting : null;
    }
}
