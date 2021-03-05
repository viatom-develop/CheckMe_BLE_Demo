package com.viatom.checkme.ble.format

import com.vaca.x1.utils.toUInt
import com.viatom.checkme.bean.DlcBean
import java.util.*

object DlcFile {
    @ExperimentalUnsignedTypes
    class DlcInfo constructor(var bytes: ByteArray) {
        var size: Int = bytes.size / 17
        var dlc: ArrayList<DlcBean> = arrayListOf<DlcBean>()


        init {

            var start: Int
            for (k in 0 until size) {
                start = k * 17
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
                dlc.add(DlcBean())
                dlc[k].apply {
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
                    hr = toUInt(setRange(start + 7, 2))
                    eface = toUInt(setRange(start + 9, 1))
                    oxy = toUInt(setRange(start + 10, 1))
                    pi = toUInt(setRange(start + 11, 1))
                    oface = toUInt(setRange(start + 12, 1))
                    prFlag = toUInt(setRange(start + 13, 1))
                    pr = toUInt(setRange(start + 14, 1))
                    bpiFace = toUInt(setRange(start + 15, 1))
                    voice = toUInt(setRange(start + 16, 1))
                }


            }


        }

        private fun setRange(start: Int, len: Int): ByteArray {
            return bytes.copyOfRange(start, start + len)
        }


    }

}