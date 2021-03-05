package com.viatom.checkme.ble.format

import com.vaca.x1.utils.toUInt
import com.viatom.checkme.bean.DlcBean
import com.viatom.checkme.bean.EcgBean
import java.util.*
import kotlin.collections.ArrayList

object EcgFile {
    @ExperimentalUnsignedTypes
    class EcgInfo constructor(var bytes: ByteArray){
        var size: Int = bytes.size/10
        var ecg:ArrayList<EcgBean> = arrayListOf<EcgBean>()


        init {

            var start:Int
            for(k in 0 until size){
                start=k*10
                val year:Int= toUInt(setRange(start , 2))
                val month:Int= toUInt(setRange(start +2, 1))-1
                val date:Int= toUInt(setRange(start + 3, 1))
                val hour:Int= toUInt(setRange(start + 4, 1))
                val minute:Int= toUInt(setRange(start + 5, 1))
                val second:Int= toUInt(setRange(start + 6, 1))
                val calendar = Calendar.getInstance()
                calendar[Calendar.YEAR] = year
                calendar[Calendar.MONTH] = month
                calendar[Calendar.DATE] = date
                calendar[Calendar.HOUR] = hour
                calendar[Calendar.MINUTE] = minute
                calendar[Calendar.SECOND] = second
                ecg.add(EcgBean())
                ecg[k].apply {
                    this.date=calendar.time
                    timeString=String.format("%04d%02d%02d%02d%02d%02d",year,month+1,date,hour,minute,second)
                    way=toUInt(setRange(start + 7, 1))
                    face=toUInt(setRange(start + 8, 1))
                    voice=toUInt(setRange(start +9, 1))
                }


            }




        }
        private fun setRange(start: Int, len: Int):ByteArray{
            return bytes.copyOfRange(start, start + len)
        }


    }

}