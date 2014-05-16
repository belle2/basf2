package b2daq.dqm.io;

import b2daq.dqm.core.Histo;
import b2daq.dqm.graphics.Canvas;
import b2daq.dqm.graphics.GMonObject;
import b2daq.dqm.ui.CanvasPanel;

public class RootFileWriter {
	
	public static String convert(CanvasPanel panel) {
		Canvas canvas = panel.getCanvas();
		StringBuffer buffer = new StringBuffer();
		buffer.append("{\n");
		buffer.append("  gROOT->SetStyle(\"Plain\");\n");
		buffer.append("  gStyle->SetOptStat(0);\n");
		buffer.append("  TCanvas* " + canvas.getName()+" = new TCanvas(\"" + canvas.getName()+
				"\", \"" + canvas.getName()+"\", 0, 0, "+panel.getWidth()+", "+panel.getHeight()+");\n");
		int drawed_histos = 0;
		for ( GMonObject obj : canvas.getMonObjects() ) {
			try {
				Histo h = (Histo)obj.get();
				if ( h.getDataType().matches("H1.") ) {
					String name = h.getName();
					buffer.append("  TH1* " + name + " = new TH1D(\""+name+"\", \"" + 
							h.getTitle()+";"+h.getAxisX().getTitle()+";"+h.getAxisY().getTitle()+
							"\" ,"+h.getAxisX().getNbins()+","+h.getAxisX().getMin()+","+h.getAxisX().getMax()+");\n");
					buffer.append("  "+name+"->GetXaxis()->CenterTitle(true);\n");
					buffer.append("  "+name+"->GetYaxis()->CenterTitle(true);\n");
					buffer.append("  "+name+"->SetMaximum("+canvas.getAxisY().get().getMax()+");\n");
					buffer.append("  "+name+"->SetMinimum("+canvas.getAxisY().get().getMin()+");\n");
					for ( int n = 0; n < h.getAxisX().getNbins(); n++ ) {
						buffer.append("  "+name+"->SetBinContent(" + (n + 1) + ", " + h.getBinContent(n) + ");\n");
					}
					if ( drawed_histos == 0 ) {
						buffer.append("  "+name+"->Draw();\n");
					} else {
						buffer.append("  "+name+"->Draw(\"same\");\n");
						drawed_histos++;
					}
				}
			} catch (Exception e) {}
		}
		buffer.append("}");
		return buffer.toString();
	}
	
}
