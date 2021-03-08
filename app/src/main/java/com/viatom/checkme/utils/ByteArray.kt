package com.viatom.checkme.utils


fun add(ori: ByteArray?, add: ByteArray): ByteArray {
    if (ori == null) {
        return add
    }

    val new: ByteArray = ByteArray(ori.size + add.size)
    for ((index, value) in ori.withIndex()) {
        new[index] = value
    }

    for ((index, value) in add.withIndex()) {
        new[index + ori.size] = value
    }

    return new
}


fun toUInt(bytes: ByteArray): Int {
    var result = 0
    for ((i, v) in bytes.withIndex()) {
        result += v.unsigned().shl(i * 8)
    }
    return result
}


fun Byte.unsigned(): Int = when {
    (toInt() < 0) -> 255 + toInt() + 1
    else -> toInt()
}