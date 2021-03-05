package com.viatom.checkme.bean

import java.util.*

data class DlcBean(
    var date: Date = Date(),
    var timeString: String = "",
    var hr: Int = 0,
    var eface: Int = 0,
    var oxy: Int = 0,
    var pi: Int = 0,
    var oface: Int = 0,
    var prFlag: Int = 0,
    var pr: Int = 0,
    var bpiFace: Int = 0,
    var voice: Int = 0
)
