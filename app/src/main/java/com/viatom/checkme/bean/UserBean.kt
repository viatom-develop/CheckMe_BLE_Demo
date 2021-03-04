package com.viatom.checkme.bean

import android.bluetooth.BluetoothDevice
import java.util.*

data class UserBean(var id:String="",var name:String="",var ico:Int=0,var sex:Int=0,var birthday:Date=Date(1997,3,22),var weight:Int=0,var height:Int=0,var pacemakeflag:Int=0,var medicalId:String="")
