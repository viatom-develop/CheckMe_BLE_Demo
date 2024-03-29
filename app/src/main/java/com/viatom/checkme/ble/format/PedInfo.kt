package com.viatom.checkme.ble.format

import android.util.Log
import com.viatom.checkme.bean.PedBean
import com.viatom.checkme.utils.toUInt
import java.util.*


class PedInfo constructor(var bytes: ByteArray) {
    var size: Int = bytes.size / 29
    var Ped: ArrayList<PedBean> = arrayListOf<PedBean>()


    init {
        Log.e("pedoSize",bytes.size.toString())
        var start: Int
        for (k in 0 until size) {
            start = k * 29
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
            Ped.add(PedBean())
            Ped[k].apply {
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
                step = toUInt(setRange(start + 7, 4))
                dis = toUInt(setRange(start + 11, 4)).toFloat() / 100f
                speed = toUInt(setRange(start + 15, 4)).toFloat() / 10f
                cal = toUInt(setRange(start + 19, 4)).toFloat() / 100f
                fat = toUInt(setRange(start + 23, 2)).toFloat() / 100f
                time = toUInt(setRange(start + 25, 2))
            }
        }


    }

    private fun setRange(start: Int, len: Int): ByteArray {
        return bytes.copyOfRange(start, start + len)
    }


}

