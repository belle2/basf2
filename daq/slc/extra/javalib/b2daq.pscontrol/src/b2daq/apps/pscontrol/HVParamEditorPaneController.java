/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.database.ConfigObject;
import b2daq.hvcontrol.core.HVCommand;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.RadioButton;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.control.TableView;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HVParamEditorPaneController implements Initializable, NSMObserver {

    @FXML
    private VBox pane;
    @FXML
    private ComboBox combo;
    @FXML
    private Button add_button;
    @FXML
    private Button create_button;
    @FXML
    private Label label_message;
    @FXML
    private ChoiceBox choice_valueset;
    @FXML
    private RadioButton before_radio;
    @FXML
    private RadioButton after_radio;
    @FXML
    private Button button_multiedit;
    @FXML
    private TabPane tabpane;
    private ObservableList<TableView> table_v = FXCollections.observableArrayList();

    public ObservableList<TableView> getTableList() {
        return table_v;
    }
    private final HashMap<String, ConfigObject> cobj_m = new HashMap<>();
    private String nodename = "";

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        combo.getSelectionModel().selectedItemProperty().addListener(new ChangeListener<String>() {
            @Override
            public void changed(ObservableValue<? extends String> selected, String oldValue, String newValue) {
                if (combo.getSelectionModel().isEmpty()
                        || newValue == null
                        || (oldValue != null && oldValue.matches(newValue))) {
                    return;
                }
                handleCombo();
            }
        });
        tabpane.setTabClosingPolicy(TabPane.TabClosingPolicy.SELECTED_TAB);
        tabpane.getSelectionModel().selectedItemProperty().addListener(
                new ChangeListener<Tab>() {
                    @Override
                    public void changed(ObservableValue<? extends Tab> ov, Tab t, Tab t1) {
                        create_button.setDisable(tabpane.getTabs().size() == 0);
                    }
                }
        );
    }

    @FXML
    public void handleCombo() {
        String configname = (String) combo.getSelectionModel().getSelectedItem();
        if (!cobj_m.containsKey(configname)) {
            NSMListenerService.requestDBGet(nodename, "", configname, 0);
            combo.setDisable(true);
            tabpane.setDisable(true);
        } else {
            update(cobj_m.get(configname));
        }
    }

    @FXML
    public void handleAddButon() {
        String configname = tabpane.getSelectionModel().getSelectedItem().getText();
        ConfigObject cobj = cobj_m.get((String) combo.getSelectionModel().getSelectedItem());
        if (cobj == null) {
            return;
        }
        for (ConfigObject valueset : cobj.getObjects("valueset")) {
            if (valueset.getObject("value").getName().matches(configname)) {
                Tab tab = new Tab();
                tab.setText(configname + "_1");
                int pos = tabpane.getSelectionModel().getSelectedIndex();
                if (after_radio.isSelected()) {
                    pos++;
                }
                addTab(tab, cobj.getObjects("channel"), valueset, pos);
                break;
            }
        }
    }

    @FXML
    public void handleReloadButon() {
        NSMListenerService.requestList(nodename);
        combo.setDisable(true);
    }

    @FXML
    public void handleMultieditButton() {
        PowerSettingDialog.showDialog(tabpane.getScene(), this);
    }
    
    @FXML
    public void handleCreateButon() {
        String [] namelist = new String [tabpane.getTabs().size()];
        for (int n = 0; n < namelist.length; n++) {
            namelist[n] = tabpane.getTabs().get(n).getText();
        }
        String [] newnamelist = CreatingNewConfigDialog.showDialog(tabpane.getScene(), 
                (String)combo.getSelectionModel().getSelectedItem(), namelist);
        if (newnamelist == null) return;
        ObservableList<HVStatusProperty> pro_m = ((TableView)tabpane.getTabs().get(0).getContent()).getItems();
        ConfigObject cobj = cobj_m.get((String) combo.getSelectionModel().getSelectedItem());
        ConfigObject cobj_out = new ConfigObject();
        cobj_out.setName(newnamelist[0]);
        cobj_out.setNode(cobj.getNode());
        cobj_out.setRevision(cobj.getRevision());
        cobj_out.setTable(cobj.getTable());
        cobj_out.addBool("sequential", true);
        cobj_out.addBool("used", true);
        for (HVStatusProperty pro: pro_m) {
            ConfigObject ccobj_out = new ConfigObject();
            ccobj_out.setNode(cobj.getObject("channel").getNode());
            ccobj_out.setRevision(cobj.getObject("channel").getRevision());
            ccobj_out.setTable(cobj.getObject("channel").getTable());
            ccobj_out.setIndex(pro.getIndex());
            ccobj_out.addInt("crate", pro.getCrate());
            ccobj_out.addInt("slot", pro.getSlot());
            ccobj_out.addInt("channel", pro.getChannel());
            ccobj_out.addBool("turnon", pro.getTurnon());
            ccobj_out.setName(newnamelist[0]);
            cobj_out.addObject("channel", ccobj_out);
        }
        for (int n = 1; n < newnamelist.length; n++) {
            pro_m = ((TableView)tabpane.getTabs().get(n-1).getContent()).getItems();
            ConfigObject ccobj_out = new ConfigObject();
            ccobj_out.setNode(cobj.getObject("valueset").getNode());
            ccobj_out.setRevision(cobj.getObject("valueset").getRevision());
            ccobj_out.setTable(cobj.getObject("valueset").getTable());
            for (HVStatusProperty pro: pro_m) {
                ConfigObject cccobj_out = new ConfigObject();
                cccobj_out.setNode(cobj.getObject("valueset").getObject("value").getNode());
                cccobj_out.setRevision(cobj.getObject("valueset").getObject("value").getRevision());
                cccobj_out.setTable(cobj.getObject("valueset").getObject("value").getTable());
                cccobj_out.setIndex(pro.getIndex());
                cccobj_out.addFloat("rampup_speed", pro.getRampupSpeed());
                cccobj_out.addFloat("rampup_speed", pro.getRampdownSpeed());
                cccobj_out.addFloat("voltage_limit", pro.getVoltageLimit());
                cccobj_out.addFloat("current_limit", pro.getCurrentLimit());
                cccobj_out.addFloat("voltage_demand", pro.getVoltageDemand());
                cccobj_out.setName(newnamelist[n]);
                ccobj_out.addObject("value", cccobj_out);
            }
            ccobj_out.setName(newnamelist[0]);
            cobj_out.addObject("valueset", ccobj_out);
        }
        cobj_out.print();
        NSMListenerService.requestDBSet(cobj_out);
    }

    @Override
    public void handleOnConnected() {
        pane.setDisable(false);
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        if (msg == null) {
            return;
        }
        HVCommand command = new HVCommand();
        command.copy(msg.getReqName());
        if (command.equals(NSMCommand.LISTSET)) {
            String[] str = msg.getData().split("\n");
            String selected = "";
            if (combo.getItems().size() > 0) {
                selected = (String) combo.getSelectionModel().getSelectedItem();
            }
            combo.getItems().clear();
            for (String s : str) {
                combo.getItems().add(s);
                combo.setDisable(false);
            }
            if (selected.length() > 0) {
                combo.getSelectionModel().select(selected);
            }
        } else if (command.equals(NSMCommand.DBSET)) {
            nodename = msg.getNodeName();
            ConfigObject cobj = NSMListenerService.getDB(msg.getNodeName());
            if (cobj != null) {
                if (!cobj_m.containsKey(cobj.getName())) {
                    ConfigObject obj = new ConfigObject();
                    msg.getData(obj);
                    cobj_m.put(obj.getName(), obj);
                }
                update(cobj);
            }
        }
    }

    @Override
    public void handleOnDisConnected() {
        pane.setDisable(true);
    }

    @Override
    public void log(LogMessage log) {
    }

    private void update(ConfigObject cobj) {
        if (cobj != null) {
            if (combo.getItems().size() == 0) {
                combo.getItems().add(cobj.getName());
                combo.getSelectionModel().select(0);
            }
            if (cobj.hasObject("channel")) {
                tabpane.getTabs().clear();
                ArrayList<ConfigObject> channels = cobj.getObjects("channel");
                table_v.clear();
                choice_valueset.getItems().clear();
                for (ConfigObject valueset : cobj.getObjects("valueset")) {
                    Tab tab = new Tab();
                    tab.setText(valueset.getObject("value").getName());
                    addTab(tab, channels, valueset, -1);
                }
                combo.setDisable(false);
                tabpane.setDisable(false);
                add_button.setDisable(false);
                choice_valueset.setDisable(false);
            }
        }
    }

    private void addTab(Tab tab, ArrayList<ConfigObject> channels, ConfigObject valueset, int pos) {
        choice_valueset.getItems().add(tab.getText());
        TableView table = HVParamTableView.create(channels, valueset, this);
        table_v.add(table);
        tab.setContent(table);
        tab.setClosable(true);
        tab.closableProperty().set(true);
        tab.setOnClosed(new EventHandler() {
            @Override
            public void handle(Event event) {
                combo.setDisable(false);
                combo.getSelectionModel().clearSelection();
                table_v.remove(tab.getContent());
                if (tabpane.getTabs().size() < 2) {
                    create_button.setDisable(true);
                }
            }
        });
        tab.setOnSelectionChanged(new EventHandler() {
            @Override
            public void handle(Event event) {
                int index = tabpane.getSelectionModel().getSelectedIndex();
                choice_valueset.getSelectionModel().select(index);
                before_radio.setDisable(index == 0);
                after_radio.setDisable(!(index < tabpane.getTabs().size() - 1));
            }
        });
        if (pos == -1) {
            tabpane.getTabs().add(tab);
        } else {
            tabpane.getTabs().add(pos, tab);
        }
    }

     public ConfigObject getConfig() {
        return cobj_m.get((String) combo.getSelectionModel().getSelectedItem());
    }

    Iterable<Tab> getTabs() {
        return tabpane.getTabs();
    }
}
