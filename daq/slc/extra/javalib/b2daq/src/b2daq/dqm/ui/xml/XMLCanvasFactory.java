package b2daq.dqm.ui.xml;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.dqm.ui.property.GPadProperty;
import b2daq.dqm.ui.property.GGraph1Property;
import b2daq.dqm.ui.property.GCircleProperty;
import b2daq.dqm.ui.property.GAxisYProperty;
import b2daq.dqm.ui.property.GAxisY2Property;
import b2daq.dqm.ui.property.GLegendProperty;
import b2daq.dqm.ui.property.GAxisXProperty;
import b2daq.dqm.ui.property.GProperty;
import b2daq.dqm.ui.property.GTextProperty;
import b2daq.dqm.ui.property.CanvasProperty;
import b2daq.dqm.ui.property.GRectProperty;
import b2daq.dqm.ui.property.GHistoProperty;
import b2daq.dqm.ui.property.GColorAxisProperty;
import java.util.ArrayList;
import javafx.scene.layout.Pane;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class XMLCanvasFactory {

    private HistoPackage _pack = null;
    private HistogramCanvas _canvas;
    private ArrayList<GHistoProperty> _histo_pro_v = new ArrayList<GHistoProperty>();

    public XMLCanvasFactory(HistoPackage pack) {
        _pack = pack;
    }

    public Pane getCanvas(Element el) {
        CanvasPanel panel = new CanvasPanel();
        _canvas = panel.getCanvas();
        GProperty pro = getProperty(el);

        if (pro != null && pro.getType().matches(CanvasProperty.ELEMENT_LABEL)) {
            ((CanvasProperty) pro).setup(_pack);
        }
        return panel;
    }

    public GProperty getProperty(Element el) {
        GProperty pro = null;
        String node_name = el.getNodeName();
        switch (node_name) {
            case CanvasProperty.ELEMENT_LABEL:
                pro = new CanvasProperty(_canvas);
                break;
            case GHistoProperty.ELEMENT_LABEL:
                pro = new GHistoProperty(_canvas, _pack, el.getAttribute("name"));
                _histo_pro_v.add((GHistoProperty) pro);
                break;
            case GGraph1Property.ELEMENT_LABEL:
                pro = new GGraph1Property(_canvas);
                break;
            case GLegendProperty.ELEMENT_LABEL:
                pro = new GLegendProperty(_canvas);
                break;
            case GPadProperty.ELEMENT_LABEL:
                pro = new GPadProperty(_canvas);
                break;
            case GAxisXProperty.ELEMENT_LABEL:
                pro = new GAxisXProperty(_canvas);
                break;
            case GAxisYProperty.ELEMENT_LABEL:
                pro = new GAxisYProperty(_canvas);
                break;
            case GAxisY2Property.ELEMENT_LABEL:
                pro = new GAxisY2Property(_canvas);
                break;
            case GColorAxisProperty.ELEMENT_LABEL:
                pro = new GColorAxisProperty(_canvas);
                break;
            case GRectProperty.ELEMENT_LABEL:
                pro = new GRectProperty(_canvas);
                break;
            case GTextProperty.ELEMENT_LABEL:
                pro = new GTextProperty(_canvas);
                break;
            case GCircleProperty.ELEMENT_LABEL:
                pro = new GCircleProperty(_canvas);
                break;
        }
        if (pro != null) {
            pro.put("value", el.getTextContent().replaceAll("\t", "").replaceAll("\n", ""));
            NamedNodeMap node_map = el.getAttributes();
            for (int m = 0; m < node_map.getLength(); m++) {
                Node node = node_map.item(m);
                pro.put(node.getNodeName(), node.getTextContent());
            }
            NodeList node_list = el.getChildNodes();
            for (int m = 0; m < node_list.getLength(); m++) {
                if (node_list.item(m).getNodeType() == Node.ELEMENT_NODE) {
                    Element el_ch = (Element) node_list.item(m);
                    GProperty pro_ch = getProperty(el_ch);
                    if (pro_ch == null) {
                        pro.put(el_ch.getNodeName(), el_ch.getTextContent());
                    } else {
                        pro.put(pro_ch);
                    }
                }
            }
        }
        return pro;
    }

    public ArrayList<GHistoProperty> getHistoProperties() {
        return _histo_pro_v;
    }

}
