/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.runcontrol.core.RCCommand;
import java.util.HashMap;
import javafx.scene.Node;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;

/**
 *
 * @author tkonno
 */
public class ConfigMainPane extends TabPane implements NSMObserver {

    private HashMap<String, String> confMap = new HashMap<>();

    @Override
    public void handleOnConnected() {
        setDisable(false);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
            return;
        }
        try {
            RCCommand command = new RCCommand();
            command.copy(msg.getReqName());
            ConfigObject obj = NSMListenerService.getDB(msg.getNodeName());
            if (command.equals(NSMCommand.DBSET)) {
                if (confMap.containsKey(obj.getNode())
                        && confMap.get(obj.getNode()).matches(obj.getName())) {
                    return;
                } else {
                    confMap.put(obj.getNode(), obj.getName());
                }
                ConfigEditorPane cpane = null;
                ConfigManager obs = null;
                for (Tab tab : getTabs()) {
                    if (tab.getText().matches(obj.getNode())) {
                        cpane = (ConfigEditorPane) tab.getContent();
                        if (cpane.boxConfigPropery().getChildren().size() > 0) {
                            if (cpane.boxConfigPropery().getChildren().get(0) instanceof RunControlConfigEditorPane) {
                                obs = (RunControlConfigEditorPane)cpane.boxConfigPropery().getChildren().get(0);
                            } else if (cpane.boxConfigPropery().getChildren().get(0) instanceof COPPERConfigEditorPane) {
                                obs = (COPPERConfigEditorPane)cpane.boxConfigPropery().getChildren().get(0);
                            } else if (cpane.boxConfigPropery().getChildren().get(0) instanceof ROPCConfigEditorPane) {
                                obs = (ROPCConfigEditorPane)cpane.boxConfigPropery().getChildren().get(0);
                            } else if (cpane.boxConfigPropery().getChildren().get(0) instanceof TTDConfigEditorPane) {
                                obs = (TTDConfigEditorPane)cpane.boxConfigPropery().getChildren().get(0);
                            }
                        }
                        break;
                    }
                }
                if (cpane == null) {
                    cpane = new ConfigEditorPane();
                    Tab tab = new Tab();
                    tab.setText(obj.getNode());
                    tab.setContent(cpane);
                    tab.setClosable(false);
                    getTabs().add(tab);
                    if (obj.getTable().matches("runcontrol")) {
                        RunControlConfigEditorPane rcpane = new RunControlConfigEditorPane();
                        obs = rcpane;
                        cpane.setPane(obj.getNode(), rcpane, rcpane);
                    } else if (obj.getTable().matches("copper")) {
                        COPPERConfigEditorPane cprpane = new COPPERConfigEditorPane();
                        obs = cprpane;
                        cpane.setPane(obj.getNode(), cprpane, cprpane);
                    } else if (obj.getTable().matches("ropc")) {
                        ROPCConfigEditorPane ropcpane = new ROPCConfigEditorPane();
                        obs = ropcpane;
                        cpane.setPane(obj.getNode(), ropcpane, ropcpane);
                    } else if (obj.getTable().matches("ttd")) {
                        TTDConfigEditorPane ttdpane = new TTDConfigEditorPane(obj.getNode());
                        obs = ttdpane;
                        cpane.setPane(obj.getNode(), ttdpane, ttdpane);
                    }
                    NSMListenerService.requestList(obj.getNode());
                }
                if (obs != null) {
                    obs.reload(msg);
                }
                cpane.labelConfigIdPropery().setText("" + obj.getId());
                cpane.labelDBTablePropery().setText(obj.getTable());
            } else if (command.equals(NSMCommand.LISTSET)) {
                if (confMap.containsKey(msg.getNodeName())) {
                    ConfigEditorPane cpane = null;
                    for (Tab tab : getTabs()) {
                        cpane = (ConfigEditorPane) tab.getContent();
                        if (cpane.boxConfigPropery().getChildren().size() > 0
                                && cpane.boxConfigPropery().getChildren().get(0) instanceof RunControlConfigEditorPane) {
                            RunControlConfigEditorPane rcpane = (RunControlConfigEditorPane) cpane.boxConfigPropery().getChildren().get(0);
                            for (Node node : rcpane.vboxNodesPropery().getChildren()) {
                                RunControlNodeConfigPane nodepane = (RunControlNodeConfigPane) node;
                                if (nodepane.labelNodeNamePropery().getText().matches(msg.getNodeName())) {
                                    nodepane.comboConfigPropery().getItems().clear();
                                    for (String confname : NSMListenerService.getList(msg.getNodeName())) {
                                        nodepane.comboConfigPropery().getItems().add(confname);
                                    }
                                    nodepane.comboConfigPropery().getSelectionModel().select(obj.getName());
                                }
                            }
                        }
                        if (tab.getText().matches(msg.getNodeName())) {
                            cpane.comboConfigNamePropery().getItems().clear();
                            for (String confname : NSMListenerService.getList(msg.getNodeName())) {
                                cpane.comboConfigNamePropery().getItems().add(confname);
                            }
                            if (obj != null) {
                                cpane.comboConfigNamePropery().getSelectionModel().select(obj.getName());
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void handleOnDisConnected() {
        setDisable(true);
    }

    @Override
    public void log(LogMessage log) {
        throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

}
