package com.viatom.checkme.bean

import java.util.*

data class TmpBean(
    var date: Date = Date(),
    var timeString: String = "",
    var way: Int = 0,
    var tmp: Float = 0f,
    var face: Int = 0
)
