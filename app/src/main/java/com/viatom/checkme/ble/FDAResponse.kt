package com.viatom.checkme.ble

import android.os.Parcelable

import com.vaca.x1.utils.toUInt

import kotlinx.android.parcel.Parcelize

object FDAResponse {


    @ExperimentalUnsignedTypes
    class CheckMeResponse  constructor(var bytes: ByteArray)  {
        var cmd: Int
        var pkgNo: Int
        var len: Int
        var content: ByteArray

        init {
            cmd = (bytes[1].toUInt() and 0xFFu).toInt()
            pkgNo = toUInt(bytes.copyOfRange(3, 5))
            len = toUInt(bytes.copyOfRange(5, 7))
            content = bytes.copyOfRange(7, 7+len)
        }
    }


}