package com.vaca.x1

import android.os.Parcelable
import com.vaca.x1.utils.toUInt
import com.viatom.checkme.bean.UserBean

import kotlinx.android.parcel.Parcelize
import java.util.*

object UserFile {
    class UserInfo constructor(var bytes: ByteArray){
        var size: Int
        var user:Array<UserBean>
        init {
            size=bytes.size/52
            user= Array(size){
              UserBean()
            }

           var start:Int
            for(k in 0 until size){
                start=k*52
                user[k].id=bytes[start].toUByte().toInt().toString()
                user[k].name=String(SetRange(start+1,16))
                user[k].ico=bytes[start+17].toUByte().toInt()
                user[k].sex=bytes[start+18].toUByte().toInt()
                val year:Int= toUInt(SetRange(start+19,2))-1900
                val month:Int= toUInt(SetRange(start+21,1))-1
                val date:Int= toUInt(SetRange(start+22,1))
                user[k].birthday= Date(year,month, date)
                user[k].weight=toUInt(SetRange(start+23,2))/200
                user[k].height=toUInt(SetRange(start+25,2))/200
                user[k].pacemakeflag=toUInt(SetRange(start+27,1))
                user[k].medicalId=String(SetRange(start+28,19))

            }




        }
        fun SetRange(start:Int,len:Int):ByteArray{
            return bytes.copyOfRange(start,start+len)
        }


    }

}