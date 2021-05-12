package com.viatom.checkme.leftnavi.wave;

import android.content.Context;


import com.viatom.checkme.utils.Constant;

import java.io.Serializable;

/**
 * ECG internal item, contains ECG data and results.
 * @author zouhao
 */
public class ECGInnerItem implements Serializable{
	
	private static final long serialVersionUID = 3176519038204466528L;
	
	// ECG data, used to draw wave
	private int[] ecgData;
	// Heart rate data
	private int[] hrData;
	// Average HR value
	private int hr;
	// ST value
	private int st;
	// QRS value
	private int qrs;
	// PVCs number
	private int pvcs;
	// QTC value
	private int qtc;
	//Diagnostic results
	private int strResultIndex;
	//Measuring time
	private int timeLength;
	//Measuring mode
	private byte checkMode;
	//Filter mode 
	private byte filterMode;
	//QT value
	private int qt;
	//isContainQT value
	private boolean isContainQT;
	private byte[] ecgDat;
	
	public ECGInnerItem(byte[] buf) {
		timeLength = (buf[0] + buf[1]&0xFF) / 2;
		hr = ((buf[6] & 0xFF) + ((buf[7] & 0xFF) << 8));
		st = (short)((buf[8] & 0xFF) + ((buf[9] & 0xFF) << 8));
		qrs = ((buf[10] & 0xFF) + ((buf[11] & 0xFF) << 8));
		pvcs = ((buf[12] & 0xFF) + ((buf[13] & 0xFF) << 8));
		qtc = ((buf[14] & 0xFF) + ((buf[15] & 0xFF) << 8));
//		strResultIndex = ((buf[16] & 0xFF) + ((buf[17] & 0xFF) << 8));
		strResultIndex = (buf[16] & 0xFF);
		checkMode = buf[17];
		filterMode = buf[18];
		
		int dataLenght = (buf[2] & 0xFF) + ((buf[3] & 0xFF) << 8)
				+ ((buf[4] & 0xFF)<<16) + ((buf[5] & 0xFF) << 24);
		int dataPos = 19 + timeLength * 2;//19 is the head length; timeLength*2 is HR data length
		ecgData = new int[dataLenght - 2];//Ignore the last two
		//Parsing ECG data
		for (int i = dataPos; (i-dataPos) < dataLenght - 2; i += 2) {
			short tempData1 = (short) ((buf[i] & 0xFF) + ((buf[i + 1] & 0xFF) << 8));// Original value1
			short tempData2 = (short) ((buf[i + 2] & 0xFF) + ((buf[i + 3] & 0xFF) << 8));// Original value2
			short tempData3 = (short) ((tempData1 + tempData2) / 2); // Average of two original value
			addDataSample(tempData3, i - dataPos);// Add the average value to list
			addDataSample(tempData2, i - dataPos + 1);// Add the Original value2 to list
		}
		ecgDat=new byte[dataLenght];
		System.arraycopy(buf,  dataPos, ecgDat, 0, dataLenght);
	}
	public ECGInnerItem(byte[] buf, Context context) {
		timeLength = (buf[0] + buf[1]&0xFF) / 2;
		hr = ((buf[6] & 0xFF) + ((buf[7] & 0xFF) << 8));
		st = (short)((buf[8] & 0xFF) + ((buf[9] & 0xFF) << 8));
		qrs = ((buf[10] & 0xFF) + ((buf[11] & 0xFF) << 8));
		pvcs = ((buf[12] & 0xFF) + ((buf[13] & 0xFF) << 8));
		qtc = ((buf[14] & 0xFF) + ((buf[15] & 0xFF) << 8));
//		strResultIndex = ((buf[16] & 0xFF) + ((buf[17] & 0xFF) << 8));
		strResultIndex = (buf[16] & 0xFF);
		checkMode = buf[17];
		filterMode = buf[18];

			isContainQT=true;
			qt = ((buf[19] & 0xFF) + ((buf[20] & 0xFF) << 8));
			
			int dataLenght = (buf[2] & 0xFF) + ((buf[3] & 0xFF) << 8)
					+ ((buf[4] & 0xFF)<<16) + ((buf[5] & 0xFF) << 24);
			int dataPos = 19 + timeLength * 2;//19 is the head length; timeLength*2 is HR data length
			ecgData = new int[dataLenght - 2];//Ignore the last two
			//Parsing ECG data
			for (int i = dataPos; (i-dataPos) < dataLenght - 2; i += 2) {
				short tempData1 = (short) ((buf[i] & 0xFF) + ((buf[i + 1] & 0xFF) << 8));// Original value1
				short tempData2 = (short) ((buf[i + 2] & 0xFF) + ((buf[i + 3] & 0xFF) << 8));// Original value2
				short tempData3 = (short) ((tempData1 + tempData2) / 2); // Average of two original value
				addDataSample(tempData3, i - dataPos);// Add the average value to list
				addDataSample(tempData2, i - dataPos + 1);// Add the Original value2 to list
			}
			ecgDat=new byte[dataLenght];
			System.arraycopy(buf,  dataPos, ecgDat, 0, dataLenght);

	}
	
	private void addDataSample(int data, int num) {
		if (num < ecgData.length)
			ecgData[num] = data;
	}

	public int[] getECGData() {
		return ecgData;
	}

	public byte[] getEcgDat() {
		return ecgDat;
	}

	public void setEcgDat(byte[] ecgDat) {
		this.ecgDat = ecgDat;
	}

	public int getHR() {
		return hr;
	}

	public int getST() {
		return st;
	}

	public int getQRS() {
		return qrs;
	}

	public int getPVCs() {
		return pvcs;
	}

	public int getQTc() {
		return qtc;
	}

	public int getStrResultIndex() {
		return strResultIndex;
	}

	public int getTimeLength() {
		return timeLength;
	}

	public byte getCheckMode() {
		return checkMode;
	}

	public byte getFilterMode() {
		return filterMode;
	}
	public int getQT() {
		return qt;
	}

}
