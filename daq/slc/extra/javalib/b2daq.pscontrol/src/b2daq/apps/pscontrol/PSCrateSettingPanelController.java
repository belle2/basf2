/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMNode;
import b2daq.nsm.NSMObserver;
import b2daq.nsm.NSMVar;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.JavaFXBuilderFactory;
import javafx.scene.Parent;
import javafx.scene.layout.VBox;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PSCrateSettingPanelController implements NSMObserver {

    @FXML
    VBox vbox;
    private int m_crateid;

    private final ArrayList<PSChannelSettingPanelController> channel = new ArrayList();
    private NSMNode m_node;
    private String m_name = "";

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public ArrayList<PSChannelSettingPanelController> getChannels() {
        return channel;
    }

    public void setCrateId(int crateid) {
        m_crateid = crateid;
    }

    public void setNode(NSMNode node) {
        m_node = node;
    }

    public void allOn(boolean allon) {
        for (PSChannelSettingPanelController ch : channel) {
            ch.turnon(allon);
        }
    }

    @Override
    public void handleOnConnected() {
        NSMListenerService.requestVGet(m_node.getName(), "crate[" + m_crateid + "].nchannels");
        NSMListenerService.requestVGet(m_node.getName(), "crate[" + m_crateid + "].name");
    }

    @Override
    public void handleOnReceived(NSMMessage msg) {
        NSMCommand cmd = new NSMCommand(msg.getReqName());
        if (cmd.equals(NSMCommand.VSET)) {
            NSMVar var = (NSMVar) msg.getObject();
            switch (var.getType()) {
                case NSMVar.INT:
                    if (var.getName().contains("crate[" + m_crateid + "].nchannels")) {
                        try {
                            int nchannels = var.getInt();
                            for (int i = 0; i < nchannels; i++) {
                                URL location = getClass().getResource("PSChannelSettingPanel.fxml");
                                FXMLLoader loader = new FXMLLoader();
                                loader.setLocation(location);
                                loader.setBuilderFactory(new JavaFXBuilderFactory());
                                Parent root = (Parent) loader.load(location.openStream());
                                PSChannelSettingPanelController controller
                                        = ((PSChannelSettingPanelController) loader.getController());
                                vbox.getChildren().add(root);
                                controller.setNode(m_node);
                                controller.setCrateId(m_crateid);
                                controller.setIndex(i);
                                controller.handleOnConnected();
                                channel.add(controller);
                            }
                            requestUpdate();
                        } catch (IOException ex) {
                            Logger.getLogger(PSCrateSettingPanelController.class.getName()).log(Level.SEVERE, null, ex);
                        }

                    }
                    for (PSChannelSettingPanelController ch : channel) {
                        ch.handleOnReceived(msg);
                    }
                    break;
                case NSMVar.FLOAT:
                    for (PSChannelSettingPanelController ch : channel) {
                        ch.handleOnReceived(msg);
                    }
                    break;
                case NSMVar.TEXT:
                    if (var.getName().contains("crate[" + m_crateid + "].name")) {
                        m_name = var.getText();
                    } else {
                        for (PSChannelSettingPanelController ch : channel) {
                            ch.handleOnReceived(msg);
                        }
                    }
                    break;
            }
        }
    }

    public void requestUpdate() {
        for (PSChannelSettingPanelController ch : channel) {
            ch.requestUpdate();
        }
    }

    @Override
    public void handleOnDisConnected() {
    }

    @Override
    public void log(LogMessage log) {
    }

}
