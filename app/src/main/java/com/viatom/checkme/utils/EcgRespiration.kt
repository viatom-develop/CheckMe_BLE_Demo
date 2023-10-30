package com.viatom.checkme.utils

object EcgRespiration {
    init {
        System.loadLibrary("ecg_respiration")
    }

    external fun initEcgRespiration()
    external fun inputEcgPoint(data:Int):Int
}