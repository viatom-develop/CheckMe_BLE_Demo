package com.viatom.checkme.ble.format

import com.viatom.checkme.bean.GluBean
import com.viatom.checkme.utils.toUInt
import java.util.*


class GluInfo constructor(var bytes: ByteArray) {
    var size: Int = bytes.size / 32
    var Glu: ArrayList<GluBean> = arrayListOf<GluBean>()


    init {

        var start: Int
        for (k in 0 until size) {
            start = k * 32
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
            Glu.add(GluBean())
            Glu[k].apply {
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
                glu= toUInt(setRange(start+7,2)).toFloat()/10f
                note=String(setRange(start+12,20))
            }
        }


    }

    private fun setRange(start: Int, len: Int): ByteArray {
        return bytes.copyOfRange(start, start + len)
    }


}

