package com.viatom.checkme.bean

import java.util.*

data class PedBean(
    var date: Date = Date(),
    var timeString: String = "",
    var step: Int = 0,
    var dis: Int = 0,
    var speed: Int = 0,
    var cal: Int = 0,
    var fat: Int = 0,
    var time: Int = 0
)
