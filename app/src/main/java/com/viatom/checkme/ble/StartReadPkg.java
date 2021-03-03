package com.viatom.checkme.ble;


import com.viatom.checkme.utils.CRCUtils;
import com.viatom.checkme.utils.LogUtils;

public class StartReadPkg {
	private byte[] buf;
	
	public StartReadPkg(String fileName){
		if(fileName==null || fileName.length()>BTConstant.BT_READ_FILE_NAME_MAX_LENGTH){
			LogUtils.d("File name error");
			return;
		}
		//+1 as character '\0'
		buf = new byte[BTConstant.COMMON_PKG_LENGTH + fileName.length() + 1];
		buf[0] = (byte)0xAA;
		buf[1] = BTConstant.CMD_WORD_START_READ;
		buf[2] = ~BTConstant.CMD_WORD_START_READ;
		buf[3] = 0;//Package number, the default is 0
		buf[4] = 0;
		buf[5] = (byte)(buf.length - BTConstant.COMMON_PKG_LENGTH);//data chunk size
		buf[6] = (byte)((buf.length - BTConstant.COMMON_PKG_LENGTH)>>8);
		
		char[] tempFileName = fileName.toCharArray();
		for (int i = 0; i < tempFileName.length; i++) {
			buf[i+7] = (byte)tempFileName[i];
		}
		buf[buf.length-1] = CRCUtils.calCRC8(buf);
	}

	public byte[] getBuf() {
		return buf;
	}
	
}
