package com.viatom.checkme.bean

import java.util.*

data class UserBean(
    var id: String = "",
    var name: String = "",
    var ico: Int = 0,
    var sex: Int = 0,
    var birthday: Date = Date(),
    var weight: Int = 0,
    var height: Int = 0,
    var color: Int = 0
)
