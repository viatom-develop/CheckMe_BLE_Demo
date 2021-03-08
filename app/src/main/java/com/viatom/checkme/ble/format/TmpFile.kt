package com.viatom.checkme.ble.format

import com.vaca.x1.utils.toUInt
import com.viatom.checkme.bean.TmpBean
import java.util.*

object TmpFile {
    @ExperimentalUnsignedTypes
    class TmpInfo constructor(var bytes: ByteArray) {
        var size: Int = bytes.size / 11
        var Tmp: ArrayList<TmpBean> = arrayListOf<TmpBean>()


        init {

            var start: Int
            for (k in 0 until size) {
                start = k * 11
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
                Tmp.add(TmpBean())
                Tmp[k].apply {
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
                    way = toUInt(setRange(start + 7, 1))
                    if (way > 2) way = 2
                    tmp = toUInt(setRange(start + 8, 2)).toFloat() / 10f
                    face = toUInt(setRange(start + 10, 1))
                    if (face > 2) face = 2
                }
            }


        }

        private fun setRange(start: Int, len: Int): ByteArray {
            return bytes.copyOfRange(start, start + len)
        }


    }

}