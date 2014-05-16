package b2rc.java.ui2;

import java.util.ArrayList;

import b2daq.core.DataObject;
import b2daq.core.RCNode;

public class RunStatus {

	private ArrayList<RCNode> _node_v = new ArrayList<RCNode>();
	private int _exp_number = 0;
	private int _cold_number = 0;
	private int _hot_number = 0;
	private int _start_time = 0;
	private int _end_time = -1;
	private int _run_config = 0;
	private String _runtype = "";
	private String _operators = "";
	private String _comment = "";

	public RunStatus() {
	}

	public int getExpNumber() {
		return _exp_number;
	}

	public int getColdNumber() {
		return _cold_number;
	}

	public int getHotNumber() {
		return _hot_number;
	}

	public int getStartTime() {
		return _start_time;
	}

	public int getEndTime() {
		return _end_time;
	}

	public int getRunConfig() {
		return _run_config;
	}

	public String getRunType() {
		return _runtype;
	}

	public String getOperators() {
		return _operators;
	}

	public String getComment() {
		return _comment;
	}

	public void setExpNumber(int exp_number) {
		_exp_number = exp_number;
	}

	public void setColdNumber(int run_number) {
		_cold_number = run_number;
	}

	public void setHotNumber(int run_number) {
		_hot_number = run_number;
	}

	public void setStartTime(int start_time) {
		_start_time = start_time;
	}

	public void setEndTime(int end_time) {
		_end_time = end_time;
	}

	public void setRunConfig(int run_config) {
		_run_config = run_config;
	}

	public void setRunType(String runtype) {
		_runtype = runtype;
	}

	public void setOperators(String operators) {
		_operators = operators;
	}

	public void setComment(String comment) {
		_comment =  comment;
	}

	public int incrementExpNumber() {
		int exp_number = getExpNumber();
		exp_number++;
		setExpNumber(exp_number);
		return exp_number;
	}

	public int incrementColdNumber() {
		int run_number = getColdNumber();
		run_number++;
		setColdNumber(run_number);
		return run_number;
	}

	public int incrementHotNumber() {
		int run_number = getHotNumber();
		run_number++;
		setHotNumber(run_number);
		return run_number;
	}

}
