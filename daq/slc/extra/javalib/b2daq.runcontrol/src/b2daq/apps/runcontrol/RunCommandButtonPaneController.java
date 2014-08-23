/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMListenerService;
import b2daq.core.LogLevel;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMMessage;
import b2daq.runcontrol.core.RCCommand;
import b2daq.runcontrol.core.RCState;
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
public class RunCommandButtonPaneController implements Initializable {

    @FXML
    private Button button_load;
    @FXML
    private Button button_start;
    @FXML
    private Button button_abort;
    @FXML
    private Button button_pause;

    private RunControlMainPaneController m_rcmain;

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
                    perform("LOAD");
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
        RCCommand cmd = new RCCommand();
        cmd.copy(reqname);
        if (cmd.equals(RCCommand.UNKNOWN)) {
            return;
        }
        String nodename =  NSMListenerService.getNSMConfig().getNsmTarget();
        NSMMessage msg = new NSMMessage(nodename, cmd);
        if (cmd.equals(RCCommand.LOAD)) {
            if (m_rcmain.getNameList() == null) {
                NSMListenerService.requestList(nodename);
                stack_wait.set(true);
                m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.DEBUG,
                        "Loading run type lists..."));
                return;
            }
            String runtype = RunConfigDialog.showDialog(
                            m_rcmain.getLabelStartTime().getScene(),
                            "Loading run configuration", "select runtype",
                            "default", m_rcmain.getNameList());
            if (runtype == null) {
                m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.DEBUG,
                        "LOAD canceled"));
                return;
            }
            m_rcmain.getLabelRunType().setText(runtype);
            msg.setData(runtype);
            msg.setNParams(1);
            msg.setParam(0, NSMCommand.DBGET.getId());
            m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.INFO,
                    "Selected run type : " + runtype));
        } else if (cmd.equals(RCCommand.START)) {
            msg.setData(m_rcmain.getRunSetting().getFieldOperator1().getText() + ":"
                    + m_rcmain.getRunSetting().getFieldOperator1().getText() + "\n"
                    + m_rcmain.getRunSetting().getAreatComments().getText().
                    replace(System.getProperty("line.separator"), "<br/>"));
            if (m_rcmain.getRunSetting().isEdited()) {
                msg.setNParams(1);
                int expno = Integer.parseInt(m_rcmain.getRunSetting().getLabelExpNo().getText());
                System.out.println(expno);
                msg.setParam(0, Integer.parseInt(m_rcmain.getRunSetting().getLabelExpNo().getText()));
            }
            m_rcmain.getLabelEndTime().setText("---- / -- / --  -- : -- : --");
        } else if (cmd.equals(RCCommand.STOP)) {
            m_rcmain.getLogView().add(new LogMessage("LOCAL", LogLevel.NOTICE,
                    "Run " + m_rcmain.getLabelRunNumbers().getText() + " ending"));
            m_rcmain.getLabelStartTime().setText(m_rcmain.getLabelStartTime().getText());
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

    public void set(RunControlMainPaneController rcmain) {
        m_rcmain = rcmain;
    }

    public void update(RCState state, boolean ready, boolean registered) {
        button_abort.setDisable(!registered);
        button_pause.setDisable(true);
        button_start.setDefaultButton(false);
        button_pause.setDefaultButton(false);
        button_load.setDefaultButton(false);
        if (state.equals(RCState.RUNNING_S)) {
            button_start.setText("STOP");
            button_start.setDisable(false);
            if (ready) {
                button_start.setDefaultButton(true);
            }
            button_load.setText("LOAD");
            button_load.setDisable(true);
            button_pause.setText("PAUSE");
            button_pause.setDisable(!registered);
        } else if (state.equals(RCState.PAUSED_S)) {
            button_start.setText("STOP");
            button_start.setDisable(false);
            if (ready) {
                button_pause.setDefaultButton(true);
            }
            button_pause.setText("RESUME");
            button_pause.setDisable(!registered);
        } else if (state.equals(RCState.READY_S)) {
            button_start.setText("START");
            button_start.setDisable(!ready);
            if (ready) {
                button_start.setDefaultButton(true);
            }
            button_load.setText("LOAD");
            button_load.setDisable(!registered);
        } else if (state.equals(RCState.NOTREADY_S)) {
            button_start.setText("START");
            button_start.setDisable(true);
            button_load.setText("LOAD");
            if (ready) {
                button_load.setDefaultButton(true);
            }
            button_load.setDisable(!registered);
        }
    }

}
