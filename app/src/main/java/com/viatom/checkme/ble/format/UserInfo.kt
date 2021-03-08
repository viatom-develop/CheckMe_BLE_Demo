package com.viatom.checkme.ble.format

import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.utils.toUInt
import com.viatom.checkme.utils.unsigned
import java.util.*


class UserInfo constructor(var bytes: ByteArray) {
    var size: Int = bytes.size / 52
    var user: Array<UserBean> = Array(size) {
        UserBean()
    }

    init {

        var start: Int
        for (k in 0 until size) {
            start = k * 52
            user[k].id = bytes[start].unsigned().toString()
            user[k].name = String(setRange(start + 1, 16))
            user[k].ico = bytes[start + 17].unsigned()
            user[k].sex = bytes[start + 18].unsigned()
            val year: Int = toUInt(setRange(start + 19, 2))
            val month: Int = toUInt(setRange(start + 21, 1)) - 1
            val date: Int = toUInt(setRange(start + 22, 1))
            val calendar = Calendar.getInstance()
            calendar[Calendar.YEAR] = year
            calendar[Calendar.MONTH] = month
            calendar[Calendar.DATE] = date
            user[k].birthday = calendar.time
            user[k].weight = toUInt(setRange(start + 23, 2)) / 200
            user[k].height = toUInt(setRange(start + 25, 2)) / 200
            user[k].pacemakeflag = toUInt(setRange(start + 27, 1))
            user[k].medicalId = String(setRange(start + 28, 19))

        }


    }

    private fun setRange(start: Int, len: Int): ByteArray {
        return bytes.copyOfRange(start, start + len)
    }


}

