/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import java.io.IOException;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Node;
import javafx.scene.control.CheckBox;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;

/**
 *
 * @author tkonno
 */
class ROPCConfigEditorPane extends VBox implements ConfigManager {

    @FXML
    private TextField fieldROPCId;
    @FXML
    private TextField fieldEB0Host;
    @FXML
    private TextField fieldEB0Port;
    @FXML
    private TextField fieldEB1TXHost;
    @FXML
    private TextField fieldEB1TXPort;
    @FXML
    private TextField fieldRecvStream1;
    @FXML
    private CheckBox checkRecvStream0;
    @FXML
    private TextField fieldRecvStream0;
    @FXML
    private VBox vboxCopperHosts;

    public TextField fieldROPCIdPropery() {
        return fieldROPCId;
    }

    public TextField fieldEB0HostPropery() {
        return fieldEB0Host;
    }

    public TextField fieldEB0PortPropery() {
        return fieldEB0Port;
    }

    public TextField fieldEB1TXHostPropery() {
        return fieldEB1TXHost;
    }

    public TextField fieldEB1TXPortPropery() {
        return fieldEB1TXPort;
    }

    public TextField fieldRecvStream1Propery() {
        return fieldRecvStream1;
    }

    public CheckBox checkRecvStream0Propery() {
        return checkRecvStream0;
    }

    public TextField fieldRecvStream0Propery() {
        return fieldRecvStream0;
    }

    public VBox vboxCopperHostsPropery() {
        return vboxCopperHosts;
    }

    public ROPCConfigEditorPane() {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("ROPCConfigEditorPane.fxml"));
        fxmlLoader.setRoot(this);
        fxmlLoader.setController(this);
        try {
            fxmlLoader.load();
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        }
        fieldRecvStream0.disableProperty().bind(checkRecvStream0.selectedProperty().not());
    }

    public void addCopperHost(ROPCCopperConfigPanel copper) {
        vboxCopperHosts.getChildren().add(copper);
        copper.fieldPortPropery().disableProperty().bind(checkRecvStream0.selectedProperty().not());
    }

    @Override
    public ConfigObject update(String nodename, String newconf) {
        ConfigObject obj = NSMListenerService.getDB(nodename);
        obj.setName(newconf);
        obj.setText("host_from", fieldEB0HostPropery().getText());
        obj.setInt("port_from", Integer.parseInt(fieldEB0PortPropery().getText()));
        obj.setText("host_to", fieldEB1TXHostPropery().getText());
        obj.setInt("port_to", Integer.parseInt(fieldEB1TXPortPropery().getText()));
        obj.setInt("ropc_nodeid", Integer.parseInt(fieldROPCIdPropery().getText()));
        obj.setText("ropc_script0", fieldRecvStream0Propery().getText());
        obj.setText("ropc_script1", fieldRecvStream1Propery().getText());
        int n = 0;
        for (Node node : vboxCopperHosts.getChildren()) {
            ROPCCopperConfigPanel copper = (ROPCCopperConfigPanel)node;
            ConfigObject cobj = (ConfigObject)obj.getObject("copper_from", n);
            cobj.setName(newconf);
            cobj.setText("hostname", copper.fieldHostNamePropery().getText());
            cobj.setInt("port", Integer.parseInt(copper.fieldPortPropery().getText()));
            cobj.setBool("used", copper.checkUsedPropery().isSelected());
            n++;
        }
        return obj;
    }

    @Override
    public void reload(NSMMessage msg) {
        ConfigObject obj = NSMListenerService.getDB(msg.getNodeName());
        fieldEB0HostPropery().setText(obj.getText("host_from"));
        fieldEB0PortPropery().setText("" + obj.getInt("port_from"));
        fieldEB1TXHostPropery().setText(obj.getText("host_to"));
        fieldEB1TXPortPropery().setText("" + obj.getInt("port_to"));
        fieldROPCIdPropery().setText("" + obj.getInt("ropc_nodeid"));
        fieldRecvStream0Propery().setText(obj.getText("ropc_script0"));
        fieldRecvStream1Propery().setText(obj.getText("ropc_script1"));
        if (obj.hasObject("copper_from")) {
            vboxCopperHosts.getChildren().clear();
            for (ConfigObject cobj : obj.getObjects("copper_from")) {
                addCopperHost(new ROPCCopperConfigPanel(cobj.getText("hostname"), cobj.getInt("port"), cobj.getBool("used")));
            }
        }
    }

}
