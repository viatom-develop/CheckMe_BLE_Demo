package com.viatom.checkme.ble.format

import com.viatom.checkme.leftnavi.wave.ECGInnerItem
import com.viatom.checkme.utils.toUInt


class EcgWaveInfo constructor(var bytes: ByteArray) {

    var hrSize: Int = toUInt(bytes.copyOfRange(0, 2))
    var waveSize: Int = toUInt(bytes.copyOfRange(2, 6)) - 4
    var hr: Int = toUInt(bytes.copyOfRange(6, 8))
    var st: Int = toUInt(bytes.copyOfRange(8, 10))
    var qrs: Int = toUInt(bytes.copyOfRange(10, 12))
    var pvcs: Int = toUInt(bytes.copyOfRange(12, 14))
    var qtc: Int = toUInt(bytes.copyOfRange(14, 16))
    var qt: Int = toUInt(bytes.copyOfRange(20, 22))
    var hrList: IntArray = IntArray(hrSize / 2)
    var waveList: IntArray = IntArray(waveSize / 2)
    var waveIntSize = waveSize / 2
    val total = 1000
    var waveViewSize = waveIntSize / total


    init {

        for (index in 0 until hrSize / 2) {
            hrList[index] = toUInt(setRange(index * 2 + 22, 2))
        }
//        for (index in 0 until waveSize / 2) {

//            waveList[index] =
//                bytes[23 + index * 2 + hrSize].toInt() * 256 + bytes[22 + index * 2 + hrSize].toInt()
//        }


        waveList= ECGInnerItem(bytes).ecgData
        waveIntSize=waveList.size
        waveViewSize=waveIntSize/total
        if (waveViewSize * total < waveIntSize) {
            waveViewSize++
        }
    }

    private fun setRange(start: Int, len: Int): ByteArray {
        return bytes.copyOfRange(start, start + len)
    }


    fun getWave(index: Int): IntArray {
        val result = IntArray(total)
        for (k in 0 until total) {
            result[k] = if (k + index * total < waveList.size) {
                waveList[k + index * total]
            } else {
                1000000
            }

        }
        return result

    }


}