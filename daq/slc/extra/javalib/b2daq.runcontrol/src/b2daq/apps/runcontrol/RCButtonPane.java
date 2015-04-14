/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMDBListSetHandler;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import b2daq.runcontrol.core.RCCommand;
import b2daq.runcontrol.core.RCState;
import java.io.IOException;
import static java.lang.Integer.MAX_VALUE;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.event.ActionEvent;
import javafx.scene.control.Button;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;

/**
 *
 * @author tkonno
 */
public class RCButtonPane extends VBox {

    private NSMNode m_node;
    private String m_config;
    private String m_table;
    private final Button m_button_configure;
    private final Button m_button_load;
    private final Button m_button_start;
    private final Button m_button_recover;
    private final Button m_button_abort;

    public RCButtonPane() {
        m_button_configure = new Button("CONFIGURE");
        m_button_configure.setMaxSize(MAX_VALUE, MAX_VALUE);
        VBox.setVgrow(m_button_configure, Priority.ALWAYS);
        m_button_configure.setOnAction((ActionEvent event) -> {
            handleConfigure();
        });
        getChildren().add(m_button_configure);
        m_button_load = new Button("LOAD");
        m_button_load.setMaxSize(MAX_VALUE, MAX_VALUE);
        VBox.setVgrow(m_button_load, Priority.ALWAYS);
        m_button_load.setOnAction((ActionEvent event) -> {
            handleLoad();
        });
        getChildren().add(m_button_load);
        m_button_start = new Button("START");
        m_button_start.setMaxSize(MAX_VALUE, MAX_VALUE);
        VBox.setVgrow(m_button_start, Priority.ALWAYS);
        m_button_start.setOnAction((ActionEvent event) -> {
            handleStart();
        });
        getChildren().add(m_button_start);
        m_button_recover = new Button("RECOVER");
        m_button_recover.setMaxSize(MAX_VALUE, MAX_VALUE);
        VBox.setVgrow(m_button_recover, Priority.ALWAYS);
        m_button_recover.setOnAction((ActionEvent event) -> {
            handleRecover();
        });
        getChildren().add(m_button_recover);
        m_button_abort = new Button("ABORT");
        m_button_abort.setMaxSize(MAX_VALUE, MAX_VALUE);
        VBox.setVgrow(m_button_abort, Priority.ALWAYS);
        m_button_abort.setOnAction((ActionEvent event) -> {
            handleAbort();
        });
        getChildren().add(m_button_abort);
    }

    public void setNode(String nodename) {
        m_node = new NSMNode(nodename);
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, m_node.getName(), "rcstate", NSMVar.TEXT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                RCState state = new RCState(var.getText());
                if (state.equals(RCState.UNKNOWN)) {
                    m_button_configure.setDisable(true);
                    m_button_load.setDisable(true);
                    m_button_start.setDisable(true);
                    m_button_recover.setDisable(true);
                    m_button_abort.setDisable(true);
                } else if (state.equals(RCState.NOTREADY_S)) {
                    m_button_configure.setDisable(false);
                    m_button_load.setDisable(false);
                    m_button_start.setDisable(true);
                    m_button_start.setText("START");
                    m_button_recover.setDisable(false);
                    m_button_abort.setDisable(false);
                } else if (state.equals(RCState.READY_S)) {
                    m_button_configure.setDisable(true);
                    m_button_load.setDisable(false);
                    m_button_start.setDisable(false);
                    m_button_start.setText("START");
                    m_button_recover.setDisable(false);
                    m_button_abort.setDisable(false);
                } else if (state.equals(RCState.RUNNING_S)) {
                    m_button_configure.setDisable(true);
                    m_button_load.setDisable(true);
                    m_button_start.setDisable(false);
                    m_button_start.setText("STOP");
                    m_button_recover.setDisable(false);
                    m_button_abort.setDisable(false);
                } else if (state.equals(RCState.RECOVERING_RS)
                        || state.equals(RCState.ABORTING_RS)) {
                    m_button_configure.setDisable(true);
                    m_button_load.setDisable(true);
                    m_button_start.setDisable(true);
                    m_button_start.setText("STOP");
                    m_button_recover.setDisable(true);
                    m_button_abort.setDisable(false);
                } else {
                    m_button_configure.setDisable(true);
                    m_button_load.setDisable(true);
                    m_button_start.setDisable(true);
                    m_button_start.setText("STOP");
                    m_button_recover.setDisable(false);
                    m_button_abort.setDisable(false);
                }
                return true;
            }
        });
    }

    public final void handleConfigure() {
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_node.getName(), "dbtable", NSMVar.TEXT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                m_table = var.getText();
                NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_node, "rcconfig", NSMVar.TEXT) {
                    @Override
                    public boolean handleVSet(NSMVar var) {
                        m_config = var.getText();
                        NSMRequestHandlerUI.get().add(new NSMDBListSetHandler(true, m_table, m_node, "RC:") {
                            @Override
                            public boolean handleDBListSet(String[] list) {
                                String conf = RunConfigDialog.showDialog(getScene(), "RC Config for " + m_node, "RC Config for " + m_node, m_config, list);
                                if (conf == null) {
                                    return true;
                                }
                                m_config = conf;
                                try {
                                    NSMCommunicator.get().request(new NSMMessage(m_node, RCCommand.CONFIGURE, m_config));
                                } catch (IOException e) {
                                    Logger.getLogger(RCButtonPane.class.getName()).log(Level.SEVERE, null, e);
                                }
                                return true;
                            }
                        });
                        return true;
                    }
                });
                return true;
            }
        });
    }

    public final void handleLoad() {
        try {
            NSMCommunicator.get().request(new NSMMessage(m_node, RCCommand.LOAD));
        } catch (IOException e) {
            Logger.getLogger(RCButtonPane.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public final void handleStart() {
        try {
            RCCommand cmd = new RCCommand();
            cmd.copy("RC_" + m_button_start.getText());
            NSMCommunicator.get().request(new NSMMessage(m_node, cmd));
        } catch (IOException e) {
            Logger.getLogger(RCButtonPane.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public final void handleRecover() {
        try {
            NSMCommunicator.get().request(new NSMMessage(m_node, RCCommand.RECOVER));
        } catch (IOException e) {
            Logger.getLogger(RCButtonPane.class.getName()).log(Level.SEVERE, null, e);
        }
    }

    public final void handleAbort() {
        try {
            NSMCommunicator.get().request(new NSMMessage(m_node, RCCommand.ABORT));
        } catch (IOException e) {
            Logger.getLogger(RCButtonPane.class.getName()).log(Level.SEVERE, null, e);
        }
    }

}
