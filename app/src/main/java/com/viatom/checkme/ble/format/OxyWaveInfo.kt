package com.viatom.checkme.ble.format

import com.viatom.checkme.utils.toUInt
import com.viatom.checkme.utils.unsigned


class OxyWaveInfo constructor(var bytes: ByteArray) {
    val size1: Int = toUInt(bytes.copyOfRange(0, 2))
    val size2: Int = toUInt(bytes.copyOfRange(2, 6))
    val o2Array = IntArray(size1 / 3)
    val pulseArray = IntArray(size1 / 3)
    val waveArray = IntArray(size2)


    init {
        for (k in 0 until size1 / 3) {
            o2Array[k] = bytes[k * 3 + 6].unsigned()
            pulseArray[k] =
                bytes[k * 3 + 7].unsigned() + bytes[k * 3 + 8].unsigned() * 256
        }
        for (k in 0 until size2) {
            waveArray[k] = bytes[k + 6 + size1].unsigned()
        }
    }


}

