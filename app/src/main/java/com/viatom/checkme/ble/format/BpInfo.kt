package com.viatom.checkme.ble.format

import com.viatom.checkme.bean.BpBean
import com.viatom.checkme.utils.toUInt
import java.util.*


class BpInfo constructor(var bytes: ByteArray) {
    var size: Int = bytes.size/11
    var Bp: ArrayList<BpBean> = arrayListOf<BpBean>()


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
            Bp.add(BpBean())
            Bp[k].apply {
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
                sys = toUInt(setRange(start + 7, 2))
                dia = toUInt(setRange(start + 9, 1))
                pr = toUInt(setRange(start + 10, 1))
            }
        }


    }

    private fun setRange(start: Int, len: Int): ByteArray {
        return bytes.copyOfRange(start, start + len)
    }


}

