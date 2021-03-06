package com.viatom.checkme.bean

import java.util.*

data class PedBean(
    var date: Date = Date(),
    var timeString: String = "",
    var step: Int = 0,
    var dis: Float = 0f,
    var speed: Float = 0f,
    var cal: Float = 0f,
    var fat: Float = 0f,
    var time: Int = 0
)
