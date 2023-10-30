package com.viatom.checkme.ble.format

import com.viatom.checkme.leftnavi.wave.ECGInnerItem
import com.viatom.checkme.utils.EcgRespiration
import com.viatom.checkme.utils.toUInt


class EcgWaveInfo constructor(var bytes: ByteArray) {

    var hrSize: Int = toUInt(bytes.copyOfRange(0, 2))
    var waveSize: Int = toUInt(bytes.copyOfRange(2, 6)) - 4
    var hr: Int = toUInt(bytes.copyOfRange(6, 8))
    var st: Int = toUInt(bytes.copyOfRange(8, 10))
    var qrs: Int = toUInt(bytes.copyOfRange(10, 12))
    var pvcs: Int = toUInt(bytes.copyOfRange(12, 14))
    var qtc: Int = toUInt(bytes.copyOfRange(14, 16))
    var result: Int = toUInt(bytes.copyOfRange(16, 17))
    var qt: Int = toUInt(bytes.copyOfRange(19, 21))
    var hrList: IntArray = IntArray(hrSize / 2)
    var waveList: IntArray = IntArray(waveSize / 2)
    var respirationArray=IntArray(waveSize/2)
    var waveIntSize = waveSize / 2
    val total = 2500
    var waveViewSize = waveIntSize / total



    init {
        for (index in 0 until hrSize / 2) {
            hrList[index] = toUInt(setRange(index * 2 + 22, 2))
        }
        waveList= ECGInnerItem(bytes).ecgData
        respirationArray=  IntArray(waveList.size)
        EcgRespiration.initEcgRespiration()
        //every 2 points
        for( i in 0 until waveList.size step 2){
            respirationArray[i]=EcgRespiration.inputEcgPoint(waveList[i])
            if(i+1<waveList.size){
                respirationArray[i+1]= respirationArray[i]
            }
        }

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

    fun getRespiration(index: Int): IntArray {
        val result = IntArray(total)
        for (k in 0 until total) {
            result[k] = if (k + index * total < respirationArray.size) {
                respirationArray[k + index * total]
            } else {
                0
            }

        }
        return result

    }


}