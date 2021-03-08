package com.viatom.checkme.ble.pkg

import com.viatom.checkme.ble.constant.BTConstant
import com.viatom.checkme.utils.CRCUtils
import kotlin.experimental.inv

class StartReadPkg(fileName: String?) {
    val buf: ByteArray = ByteArray(BTConstant.COMMON_PKG_LENGTH + fileName!!.length + 1)

    init {
        buf[0] = 0xAA.toByte()
        buf[1] = BTConstant.CMD_WORD_START_READ
        buf[2] = BTConstant.CMD_WORD_START_READ.inv()
        buf[3] = 0 //Package number, the default is 0
        buf[4] = 0
        buf[5] = (buf.size - BTConstant.COMMON_PKG_LENGTH).toByte() //data chunk size
        buf[6] = (buf.size - BTConstant.COMMON_PKG_LENGTH shr 8).toByte()
        val tempFileName = fileName!!.toCharArray()
        for (i in tempFileName.indices) {
            buf[i + 7] = tempFileName[i].toByte()
        }
        buf[buf.size - 1] = CRCUtils.calCRC8(buf)
    }
}