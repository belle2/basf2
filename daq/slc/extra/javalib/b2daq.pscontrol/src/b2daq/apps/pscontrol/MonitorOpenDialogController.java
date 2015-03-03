/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.dqm.core.TimedGraph1D;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.hvcontrol.core.HVState;
import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMObserver;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMListenerGUIHandler;
import java.net.URL;
import java.util.HashMap;
import java.util.ResourceBundle;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.paint.Color;
import javafx.stage.Modality;
import javafx.stage.Stage;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class MonitorOpenDialogController implements Initializable, NSMObserver {

    @FXML
    private Button openButton;
    @FXML
    private Button cancelButton;
    @FXML
    private ComboBox combo;

    private boolean isOpen = false;
    private PSSettingMainPanelController m_pmain;
    private CanvasPanel canvas;
    private HashMap<String, TimedGraph1D> gr = new HashMap<>();
    private Color[] color = {
        Color.BLACK, Color.RED, Color.BLUE, Color.GREEN, Color.MAGENTA, Color.LIGHTGRAY,
        Color.ORANGE, Color.LIGHTSTEELBLUE, Color.DARKSLATEBLUE, Color.ORANGERED,
        Color.BLUEVIOLET, Color.CHOCOLATE
    };

    public boolean isOpened() {
        return isOpen;
    }
    
    public void setSettingMainPanel(PSSettingMainPanelController pmain) {
        m_pmain = pmain;
        for (PSCrateSettingPanelController c : m_pmain.getCrates()) {
            String name = c.getName();
            System.out.println("konno:"+name);
            combo.getItems().add(name);
        }
    }

    @FXML
    protected void handleOpenButton() {
        try {
            String name = (String) combo.getSelectionModel().getSelectedItem();
            PSCrateSettingPanelController pcrate = m_pmain.getCrate(name);
            if (pcrate != null) {
                canvas = new CanvasPanel();
                for (PSChannelSettingPanelController ch : pcrate.getChannels()) {
                    String vname = ch.getVName("vmon");
                    TimedGraph1D g = new TimedGraph1D(vname, ";Time;Voltage [V]", 1000, 0, 3600);
                    g.setLineColor(color[gr.keySet().size()]);
                    gr.put(vname, g);
                    canvas.getCanvas().addHisto(g);
                    canvas.getCanvas().setFillColor(Color.LIGHTGRAY);
                    canvas.getCanvas().getPad().setY(0.07);
                    canvas.getCanvas().getAxisX().setNdivisions(10);
                }
                canvas.setPrefSize(520, 440);
                Scene scene = new Scene(canvas);
                scene.getStylesheets().add(LogMessage.getCSSPath());
                scene.getStylesheets().add(HVState.getCSSPath());
                Stage dialog = new Stage();//StageStyle.UTILITY);
                dialog.setScene(scene);
                dialog.initModality(Modality.WINDOW_MODAL);
                dialog.setTitle(name);
                dialog.centerOnScreen();
                dialog.show();
                NSMListenerGUIHandler.get().add(this);
            }
        } catch (Exception e) {
        }
        handleCloseAction(true);
    }

    @FXML
    protected void handleCancelButton() {
        handleCloseAction(false);
    }

    private void handleCloseAction(boolean isOpen) {
        this.isOpen = isOpen;
        openButton.getScene().getWindow().hide();
    }

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        openButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
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

    @Override
    public void handleOnConnected() {
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        NSMCommand cmd = new NSMCommand(msg.getReqName());
        if (cmd.equals(NSMCommand.VSET)) {
            NSMVar var = (NSMVar) msg.getObject();
            switch (var.getType()) {
                case NSMVar.FLOAT:
                    if (var.getName().contains("vmon")) {
                        if (!gr.containsKey(var.getName())) {
                        }
                        gr.get(var.getName()).addPoint(var.getFloat());
                        canvas.update();
                    }
                    break;
            }
        }
    }

    @Override
    public void handleOnDisConnected() {
    }

    @Override
    public void log(LogMessage log) {
    }

}
