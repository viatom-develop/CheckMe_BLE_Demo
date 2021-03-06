package com.viatom.checkme.leftnavi

import kotlinx.coroutines.channels.Channel

object UiChannel {
    val progressChannel = Channel<Int>(Channel.CONFLATED)
}