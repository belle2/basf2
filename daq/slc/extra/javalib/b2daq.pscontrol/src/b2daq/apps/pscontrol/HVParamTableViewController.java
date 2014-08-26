/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.database.ConfigObject;
import java.net.URL;
import java.util.ArrayList;
import java.util.ResourceBundle;
import javafx.beans.Observable;
import javafx.collections.FXCollections;
import javafx.collections.ListChangeListener;
import javafx.collections.ObservableList;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.CheckBoxTableCell;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.control.cell.TextFieldTableCell;
import javafx.util.Callback;
import javafx.util.StringConverter;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class HVParamTableViewController implements Initializable {

    @FXML
    private TableView table_hvconfig;
    @FXML
    private TableColumn col_turnon;
    @FXML
    private TableColumn col_rampup;
    @FXML
    private TableColumn col_rampdown;
    @FXML
    private TableColumn col_voltageLimit;
    @FXML
    private TableColumn col_currentLimit;
    @FXML
    private TableColumn col_voltageDemand;
    private HVParamEditorPaneController editor;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        col_turnon.setCellFactory(CheckBoxTableCell.forTableColumn(col_turnon));
        col_turnon.setEditable(true);
        col_turnon.setCellValueFactory(new PropertyValueFactory<>("turnon"));
        Callback<TableColumn<HVStatusProperty, Float>, TableCell<HVStatusProperty, Float>> floatCelleFactory = (TableColumn<HVStatusProperty, Float> arg0) -> new TextFieldTableCell<>(new StringConverter<Float>() {

            @Override
            public String toString(Float object) {
                return "" + object;
            }

            @Override
            public Float fromString(String string) {
                try {
                    return Float.parseFloat(string);
                } catch (Exception e) {
                    return 0f;
                }
            }
        });
        col_rampup.setCellFactory(floatCelleFactory);
        col_rampdown.setCellFactory(floatCelleFactory);
        col_voltageLimit.setCellFactory(floatCelleFactory);
        col_currentLimit.setCellFactory(floatCelleFactory);
        col_voltageDemand.setCellFactory(floatCelleFactory);
        col_rampup.setOnEditCommit(new EventHandler<TableColumn.CellEditEvent<HVStatusProperty, Float>>() {
            @Override
            public void handle(TableColumn.CellEditEvent<HVStatusProperty, Float> t) {
                for (Object item : table_hvconfig.getItems()) {
                    HVStatusProperty pro = ((HVStatusProperty) item);
                    pro.setTurnon(false);
                    System.out.println(pro.getIndex() + " " + pro.getTurnon());
                }
                ((HVStatusProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setRampupSpeed(t.getNewValue());
            }
        });
        col_rampdown.setOnEditCommit(new EventHandler<TableColumn.CellEditEvent<HVStatusProperty, Float>>() {
            @Override
            public void handle(TableColumn.CellEditEvent<HVStatusProperty, Float> t) {
                ((HVStatusProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setRampdownSpeed(t.getNewValue());
            }
        });
        col_voltageLimit.setOnEditCommit(new EventHandler<TableColumn.CellEditEvent<HVStatusProperty, Float>>() {
            @Override
            public void handle(TableColumn.CellEditEvent<HVStatusProperty, Float> t) {
                ((HVStatusProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setVoltageLimit(t.getNewValue());
            }
        });
        col_currentLimit.setOnEditCommit(new EventHandler<TableColumn.CellEditEvent<HVStatusProperty, Float>>() {
            @Override
            public void handle(TableColumn.CellEditEvent<HVStatusProperty, Float> t) {
                ((HVStatusProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setCurrentLimit(t.getNewValue());
            }
        });
        col_voltageDemand.setOnEditCommit(new EventHandler<TableColumn.CellEditEvent<HVStatusProperty, Float>>() {
            @Override
            public void handle(TableColumn.CellEditEvent<HVStatusProperty, Float> t) {
                ((HVStatusProperty) t.getTableView().getItems().get(t.getTablePosition().getRow())).setVoltageDemand(t.getNewValue());
            }
        });
        table_hvconfig.setItems(createData());
    }

    void add(ArrayList<ConfigObject> channels, ConfigObject valueset) {
        for (ConfigObject vobj : valueset.getObjects("value")) {
            ConfigObject chobj = channels.get(vobj.getIndex());
            table_hvconfig.getItems().add(new HVStatusProperty(chobj.getIndex(),
                    chobj.getInt("crate"), chobj.getInt("slot"),
                    chobj.getInt("channel"), chobj.getBool("turnon"),
                    vobj.getName(), vobj.getFloat("rampup_speed"),
                    vobj.getFloat("rampdown_speed"),
                    vobj.getFloat("voltage_limit"),
                    vobj.getFloat("current_limit"),
                    vobj.getFloat("voltage_demand")));
        }
    }

    TableView getTable() {
        return table_hvconfig;
    }

    private ObservableList<HVStatusProperty> createData() {
        ObservableList<HVStatusProperty> data = FXCollections.<HVStatusProperty>observableArrayList(
                new Callback<HVStatusProperty, Observable[]>() {
                    @Override
                    public Observable[] call(HVStatusProperty player) {
                        return new Observable[]{player.turnonProperty()};
                    }
                }
        );
        data.addListener(new ListChangeListener<HVStatusProperty>() {
            @Override
            public void onChanged(ListChangeListener.Change<? extends HVStatusProperty> change) {
                if (editor == null) {
                    return;
                }
                int counter = 0;
                while (change.next()) {
                    if (change.wasUpdated()) {
                        for (TableView table : editor.getTableList()) {
                            if (table.equals(table_hvconfig)) {
                                continue;
                            }
                            for (HVStatusProperty p : change.getList()) {
                                for (Object o : table.getItems()) {
                                    HVStatusProperty p_in = (HVStatusProperty)o;
                                    if (p_in.getIndex()==p.getIndex()) {
                                        p_in.setTurnon(p.getTurnon());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        });

        return data;
    }

    void setEditor(HVParamEditorPaneController editor) {
        this.editor = editor;
    }

}
