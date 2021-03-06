package com.viatom.checkme

import kotlinx.coroutines.channels.Channel

object Chanl {
    val teChannel = Channel<Int>(Channel.CONFLATED)
    val progressChannel = Channel<Int>(Channel.CONFLATED)
}