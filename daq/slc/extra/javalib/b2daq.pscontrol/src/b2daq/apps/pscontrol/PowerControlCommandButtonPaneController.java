/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.core.LogLevel;
import b2daq.hvcontrol.core.HVCommand;
import b2daq.hvcontrol.core.HVState;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PowerControlCommandButtonPaneController implements Initializable {

    @FXML
    private Button button_standby;
    @FXML
    private Button button_peak;
    @FXML
    private Button button_shoulder;
    @FXML
    private Button button_recover;
    @FXML
    private Button button_configure;
    @FXML
    private Button button_turnoff;

    private PowerSupplyMainPaneController m_rcmain;

    private final SimpleBooleanProperty stack_wait = new SimpleBooleanProperty(false);

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        stack_wait.addListener(new ChangeListener<Boolean>() {

            @Override
            public void changed(ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) {
                if (oldValue == true && newValue == false) {
                    perform("CONFIGURE");
                }
            }
        });
    }

    public void clearStack() {
        stack_wait.set(false);
    }

    @FXML
    private void handleButtonAction(ActionEvent event) {
        String reqname = ((Button) event.getSource()).getText();
        perform(reqname);
    }

    private void perform(String reqname) {
        HVCommand cmd = new HVCommand();
        cmd.copy(reqname);
        if (cmd.equals(HVCommand.UNKNOWN)) {
            return;
        }
        String nodename = NSMListenerService.getNSMConfig().getNsmTarget();
        NSMMessage msg = new NSMMessage(nodename, cmd);
        if (cmd.equals(HVCommand.CONFIGURE)) {
            if (m_rcmain.getNameList() == null) {
                NSMListenerService.requestList(nodename);
                stack_wait.set(true);
                m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.DEBUG,
                        "Loading run type lists..."));
                return;
            }
            String configname = PowerConfigDialog.showDialog(
                    button_standby.getScene(),
                    "Loading configuration", "select configuration",
                    "default", m_rcmain.getNameList());
            if (configname == null) {
                m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.DEBUG,
                        "LOAD canceled"));
                return;
            }
            m_rcmain.getLabelConfig().setText(configname);
            msg.setData(configname);
            m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.INFO,
                    "Selected run type : " + configname));
        }
        if (NSMListenerService.request(msg)) {
            m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.DEBUG,
                    "Command All " + reqname + ">>"
                    + nodename + ""));
        } else {
            m_rcmain.getLogView().add("Failed to send command (" + reqname + ">>"
                    + nodename + ")");
        }
    }

    public void set(PowerSupplyMainPaneController rcmain) {
        m_rcmain = rcmain;
    }

    public void update(HVState state) {
        button_turnoff.setDisable(false);
        button_recover.setDisable(true);
        if (state.equals(HVState.OFF_S)) {
            button_turnoff.setText("TURNON");
            button_standby.setDisable(true);
            button_peak.setDisable(true);
            button_shoulder.setDisable(true);
            button_configure.setDisable(false);
        } else if (state.isStable()) {
            button_turnoff.setText("TURNOFF");
            if (state.equals(HVState.PEAK_S)) {
                button_standby.setDisable(false);
                button_peak.setDisable(true);
                button_shoulder.setDisable(false);
                button_configure.setDisable(true);
            } else if (state.equals(HVState.SHOULDER_S)) {
                button_standby.setDisable(false);
                button_peak.setDisable(false);
                button_shoulder.setDisable(true);
                button_configure.setDisable(true);
            } else if (state.equals(HVState.STANDBY_S)) {
                button_standby.setDisable(true);
                button_peak.setDisable(false);
                button_shoulder.setDisable(false);
                button_configure.setDisable(false);
            }
        } else if (state.isTransition()) {
            button_turnoff.setText("TURNOFF");
            button_standby.setDisable(true);
            button_peak.setDisable(true);
            button_shoulder.setDisable(true);
            button_configure.setDisable(true);
        } else if (state.isError()) {
            button_turnoff.setText("TURNOFF");
            button_standby.setDisable(true);
            button_peak.setDisable(true);
            button_shoulder.setDisable(true);
            button_configure.setDisable(true);
        }
    }

}
