package com.viatom.checkme.ble.format

import com.vaca.x1.utils.toUInt
import com.viatom.checkme.bean.DlcBean
import java.util.*

object OxyWaveFile {
    @ExperimentalUnsignedTypes
    class OxyWaveInfo constructor(var bytes: ByteArray) {
        val size1:Int= toUInt(bytes.copyOfRange(0,2))
        val size2:Int= toUInt(bytes.copyOfRange(2,6))
        val o2Array=IntArray(size1/3)
        val pulseArray=IntArray(size1/3)
        val waveArray=IntArray(size2)



        init {
            for(k in 0 until size1/3){
                o2Array[k]= bytes[k*3+6].toUByte().toInt()
                pulseArray[k]=bytes[k*3+7].toUByte().toInt()+bytes[k*3+8].toUByte().toInt()*256
            }
            for(k in 0 until size2){
                waveArray[k]=bytes[k+6+size1].toUByte().toInt()
            }
        }
        



    }

}