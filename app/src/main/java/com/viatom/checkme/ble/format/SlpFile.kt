package com.viatom.checkme.ble.format

import com.vaca.x1.utils.toUInt
import com.viatom.checkme.bean.SlpBean
import java.util.*

object SlpFile {
    @ExperimentalUnsignedTypes
    class SlpInfo constructor(var bytes: ByteArray) {
        var size: Int = bytes.size / 18
        var Slp: ArrayList<SlpBean> = arrayListOf<SlpBean>()


        init {

            var start: Int
            for (k in 0 until size) {
                start = k * 18
                val year: Int = toUInt(setRange(start, 2))
                val month: Int = toUInt(setRange(start + 2, 1)) - 1
                val date: Int = toUInt(setRange(start + 3, 1))
                val hour: Int = toUInt(setRange(start + 4, 1))
                val minute: Int = toUInt(setRange(start + 5, 1))
                val second: Int = toUInt(setRange(start + 6, 1))
                val calendar = Calendar.getInstance()
                calendar[Calendar.YEAR] = year
                calendar[Calendar.MONTH] = month
                calendar[Calendar.DATE] = date
                calendar[Calendar.HOUR] = hour
                calendar[Calendar.MINUTE] = minute
                calendar[Calendar.SECOND] = second
                Slp.add(SlpBean())
                Slp[k].apply {
                    this.date = calendar.time
                    timeString = String.format(
                        "%04d%02d%02d%02d%02d%02d",
                        year,
                        month + 1,
                        date,
                        hour,
                        minute,
                        second
                    )
                    time = toUInt(setRange(start + 7, 4))
                    lowTime = toUInt(setRange(start + 11, 2))
                    lowCount = toUInt(setRange(start + 13, 2))
                    minO2 = toUInt(setRange(start + 15, 1))
                    meanO2 = toUInt(setRange(start + 16, 1))
                    face = toUInt(setRange(start + 17, 1))
                    if (face > 2) face = 2
                }
            }


        }

        private fun setRange(start: Int, len: Int): ByteArray {
            return bytes.copyOfRange(start, start + len)
        }


    }

}