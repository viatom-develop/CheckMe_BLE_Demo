package com.viatom.checkme.ble;


import com.viatom.checkme.utils.CRCUtils;
import com.viatom.checkme.utils.LogUtils;

public class ReadContentPkg {
	private byte[] buf;
	
	public ReadContentPkg(int pkgNum) {
		// TODO Auto-generated constructor stub
		if (pkgNum<0) {
			LogUtils.d("ReadContentPkg package number error");
			return;
		}

		buf = new byte[BTConstant.COMMON_PKG_LENGTH];
		buf[0] = (byte)0xAA;
		buf[1] = BTConstant.CMD_WORD_READ_CONTENT;
		buf[2] = ~BTConstant.CMD_WORD_READ_CONTENT;
		buf[3] = (byte)(pkgNum); //Package number
		buf[4] = (byte)(pkgNum>>8);
		buf[5] = 0; //data chunk size, the default is 0
		buf[6] = 0;
		
		buf[buf.length-1] = CRCUtils.calCRC8(buf);
	}

	public byte[] getBuf() {
		return buf;
	}
	
}
