package com.viatom.checkme.bean

import java.util.*

data class BpBean(
    var date: Date = Date(),
    var timeString: String = "",
    var sys: Int = 0,
    var dia: Int=0,
    var pr: Int = 0
)
