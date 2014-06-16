/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.ui;

import b2daq.database.DBObject;
import b2daq.database.DBObjectProperty;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.NSMData;
import b2daq.nsm.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Text;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class NetworkConfigPaneController implements Initializable, NSMObserver {

    @FXML
    private Text text_socket;
    @FXML
    private Rectangle rect_socket;
    @FXML
    private Text text_nsm;
    @FXML
    private Rectangle rect_nsm;
    @FXML
    private TextField field_socket_host;
    @FXML
    private TextField field_socket_port;
    @FXML
    private TextField field_nsm_host;
    @FXML
    private TextField field_nsm_port;
    @FXML
    private TextField field_nsm_node;
    @FXML
    private TextField field_nsm_target;
    @FXML
    private TableView table_nsm;
    @FXML
    private TableView table_db;
    @FXML
    private Button button_socket_reconnect;
    @FXML
    private Button button_nsm_reconnect;

    private NSMConfig config;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
    }
    
    @FXML
    public void handleRecnnectSocket() {
        NSMListenerService.restart();
    }
    
    public void add(NSMDataProperty pro) {
        for (Object o : table_nsm.getItems()) {
            NSMDataProperty p = (NSMDataProperty) o;
            if (p.getDataname().matches(pro.getDataname())) {
                return;
            }
        }
        table_nsm.getItems().add(pro);
    }

    public void add(DBObjectProperty pro) {
        for (Object o : table_db.getItems()) {
            DBObjectProperty p = (DBObjectProperty) o;
            if (p.getNode().matches(pro.getNode())) {
                return;
            }
        }
        table_db.getItems().add(pro);
    }

    public void add(NSMData data) {
        add(new NSMDataProperty(data.getName(),
                data.getFormat(), data.getRevision()));
    }

    public void add(DBObject obj) {
        add(new DBObjectProperty(obj.getNode(),
                obj.getTable(), obj.getName(), obj.getId()));
    }

    public TextField getSocketHost() {
        return field_socket_host;
    }

    public TextField getSocketPort() {
        return field_socket_port;
    }

    public TextField getNSMHost() {
        return field_nsm_host;
    }

    public TextField getNSMPort() {
        return field_nsm_port;
    }

    public TextField getNSMNode() {
        return field_nsm_node;
    }

    public TextField getNSMTarget() {
        return field_nsm_target;
    }

    public ObservableList<NSMDataProperty> getNSMDataProperties() {
        return table_nsm.getItems();
    }

    public ObservableList<DBObjectProperty> getDBObjectProperties() {
        return table_db.getItems();
    }

    public void addNSMData(NSMDataProperty data) {
        table_nsm.getItems().add(data);
    }

    public NSMConfig getConfig() {
        return config;
    }

    public void setConfig(NSMConfig config) {
        this.config = config;
        field_socket_host.setText(config.getHostname());
        field_socket_port.setText("" + config.getPort());
        field_nsm_host.setText(config.getNsmHost());
        field_nsm_port.setText("" + config.getNsmPort());
        field_nsm_node.setText(config.getNsmNode());
        field_nsm_target.setText(config.getNsmTarget());
        config.hostnameProperty().bind(field_socket_host.textProperty());
        field_socket_port.textProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable, Object oldValue, Object newValue) {
                try {
                    config.portProperty().set(Integer.parseInt(field_socket_port.getText()));
                } catch (Exception e) {
                }
            }
        });
        config.nsmHostProperty().bind(field_nsm_host.textProperty());
        field_nsm_port.textProperty().addListener(new ChangeListener() {
            @Override
            public void changed(ObservableValue observable, Object oldValue, Object newValue) {
                try {
                    config.nsmPortProperty().set(Integer.parseInt(field_nsm_port.getText()));
                } catch (Exception e) {
                }
            }
        });
        config.nsmNodeProperty().bind(field_nsm_node.textProperty());
        config.nsmTargetProperty().bind(field_nsm_target.textProperty());
    }

    public void setState(boolean socket_on, boolean nsm_on) {
        button_socket_reconnect.setDisable(socket_on);
        if (socket_on) {
            text_socket.setText("ONLINE");
            text_socket.setFill(Color.WHITE);
            rect_socket.setFill(Color.LIMEGREEN);
            rect_socket.setStroke(Color.LIGHTGREEN);
        } else {
            text_socket.setText("OFFLINE");
            text_socket.setFill(Color.RED);
            rect_socket.setFill(Color.WHITE);
            rect_socket.setStroke(Color.BLACK);
        }
        if (nsm_on) {
            text_nsm.setText("ONLINE");
            text_nsm.setFill(Color.WHITE);
            rect_nsm.setFill(Color.LIMEGREEN);
            rect_nsm.setStroke(Color.LIGHTGREEN);
        } else {
            text_nsm.setText("OFFLINE");
            text_nsm.setFill(Color.RED);
            rect_nsm.setFill(Color.WHITE);
            rect_nsm.setStroke(Color.BLACK);
        }
    }

    @Override
    public void handleOnConnected() {
        setState(true, true);
        for (NSMDataProperty pro : getNSMDataProperties()) {
            System.out.println(pro.getDataname()+" "+
            pro.getFormat() +" "+ pro.getRevision());
            NSMListenerService.requestNSMGet(pro.getDataname(),
            pro.getFormat(), pro.getRevision());    
        }
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
    }

    @Override
    public void handleOnDisConnected() {
        setState(false, false);
    }

    @Override
    public void log(LogMessage log) {
    }

}
