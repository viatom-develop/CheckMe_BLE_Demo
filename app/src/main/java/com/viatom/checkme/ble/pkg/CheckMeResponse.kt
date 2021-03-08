package com.viatom.checkme.ble.pkg

import com.viatom.checkme.utils.toUInt
import com.viatom.checkme.utils.unsigned


class CheckMeResponse(var bytes: ByteArray) {
    var cmd: Int = bytes[1].unsigned()
    var pkgNo: Int = toUInt(bytes.copyOfRange(3, 5))
    var len: Int = toUInt(bytes.copyOfRange(5, 7))
    var content: ByteArray = bytes.copyOfRange(7, 7 + len)

}


