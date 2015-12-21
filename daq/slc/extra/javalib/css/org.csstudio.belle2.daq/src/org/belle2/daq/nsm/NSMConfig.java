/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.nsm;

/**
 *
 * @author tkonno
 */
public class NSMConfig  {

    private String hostname = new String();
    private int port = 0;
    private String guiNode = new String();
    private String nsmHost = new String();
    private int nsmPort = 0;
    private String nsmNode = new String();

    public String hostnameProperty() {
        return hostname;
    }

    public int portProperty() {
        return port;
    }

    public String guiNodeProperty() {
        return guiNode;
    }

    public String nsmHostProperty() {
        return nsmHost;
    }

    public int nsmPortProperty() {
        return nsmPort;
    }

    public String nsmNodeProperty() {
        return nsmNode;
    }

    public String getHostname() {
        return hostname;
    }

    public int getPort() {
        return port;
    }

    public String getGuiNode() {
        return guiNode.toUpperCase();
    }

    public String getNsmHost() {
        return nsmHost;
    }

    public int getNsmPort() {
        return nsmPort;
    }

    public String getNsmNode() {
        return nsmNode.toUpperCase();
    }

    public void setHostname(String hostname) {
        this.hostname = hostname;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public void setGuimNode(String nsmNode) {
        this.guiNode = nsmNode;
    }

    public void setNsmHost(String nsmHost) {
        this.nsmHost = nsmHost;
    }

    public void setNsmPort(int nsmPort) {
        this.nsmPort = nsmPort;
    }

    public void setNsmNode(String nsmNode) {
        this.nsmNode = nsmNode;
    }

}
