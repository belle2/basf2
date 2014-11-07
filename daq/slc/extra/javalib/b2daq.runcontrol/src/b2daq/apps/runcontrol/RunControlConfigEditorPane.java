/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Node;
import javafx.scene.layout.VBox;

/**
 *
 * @author tkonno
 */
class RunControlConfigEditorPane extends VBox implements ConfigManager {

    @FXML
    private VBox vboxNodes;
    @FXML
    private VBox vboxStatus;

    public VBox vboxNodesPropery() {
        return vboxNodes;
    }

    public VBox vboxStatusPropery() {
        return vboxStatus;
    }

    public RunControlConfigEditorPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("RunControlConfigEditorPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }

    public void addNode(Node node) {
        vboxNodes.getChildren().add(node);
    }

    public void addStatus(Node node) {
        vboxStatus.getChildren().add(node);
    }

    @Override
    public ConfigObject update(String nodename, String newconf) {
        ConfigObject obj = NSMListenerService.getDB(nodename);
        obj.setName(newconf);
        int n = 0;
        for (Node node : vboxNodes.getChildren()) {
            RunControlNodeConfigPane nodepane = (RunControlNodeConfigPane) node;
            ConfigObject cobj = (ConfigObject)obj.getObject("node", n);
            cobj.setName(newconf);
            ConfigObject ccobj = (ConfigObject) cobj.getObject("runtype");
            cobj.setBool("used", nodepane.checkUsedPropery().isSelected());
            cobj.setBool("sequential", nodepane.checkSequentialPropery().isSelected());
            ccobj.setName((String)nodepane.comboConfigPropery().getSelectionModel().getSelectedItem());
            n++;
        }
        n = 0;
        for (Node node : vboxStatus.getChildren()) {
            RunControlStatusConfigPane statuspane = (RunControlStatusConfigPane) node;
            ConfigObject cobj = (ConfigObject)obj.getObject("nsmdata", n);
            cobj.setName(newconf);
            cobj.setText("hostnode", statuspane.labelNamePropery().getText());
            cobj.setText("dataname", statuspane.fieldNamePropery().getText());
            cobj.setText("format", statuspane.fieldFormatPropery().getText());
            cobj.setInt("file_revision", Integer.parseInt(statuspane.fieldRevisionPropery().getText()));
            n++;
        }
        
        return obj;
    }

    @Override
    public void reload(NSMMessage msg) {
        ConfigObject obj = NSMListenerService.getDB(msg.getNodeName());
        if (obj.hasObject("node")) {
            vboxNodes.getChildren().clear();
            for (ConfigObject cobj : obj.getObjects("node")) {
                ConfigObject ccobj = (ConfigObject) cobj.getObject("runtype");
                addNode(new RunControlNodeConfigPane(ccobj.getNode(), cobj.getBool("used"),
                        cobj.getBool("sequential"), ccobj.getName()));

            }
        }
        if (obj.hasObject("nsmdata")) {
            vboxStatus.getChildren().clear();
            for (ConfigObject cobj : obj.getObjects("nsmdata")) {
                addStatus(new RunControlStatusConfigPane(cobj.getText("hostnode"),
                        cobj.getText("dataname"), cobj.getText("format"),
                        cobj.getInt("file_revision")));

            }
        }
    }

}
