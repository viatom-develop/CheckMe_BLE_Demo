package com.viatom.checkme.ble.pkg

import com.viatom.checkme.ble.constant.BTConstant
import com.viatom.checkme.utils.CRCUtils
import kotlin.experimental.inv

class EndReadPkg {
    var buf: ByteArray = ByteArray(BTConstant.COMMON_PKG_LENGTH)

    init {
        // TODO Auto-generated constructor stub
        buf[0] = 0xAA.toByte()
        buf[1] = BTConstant.CMD_WORD_END_READ
        buf[2] = BTConstant.CMD_WORD_END_READ.inv()
        buf[3] = 0 //Package number, the default is 0
        buf[4] = 0
        buf[5] = 0 //data chunk size, the default is 0
        buf[6] = 0
        buf[buf.size - 1] = CRCUtils.calCRC8(buf)
    }
}