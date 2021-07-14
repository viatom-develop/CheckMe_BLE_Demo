package com.viatom.checkme.bean

import java.util.*

data class GluBean(
    var date: Date = Date(),
    var timeString: String = "",
    var glu:Float=0f,
    var note:String=""
)
