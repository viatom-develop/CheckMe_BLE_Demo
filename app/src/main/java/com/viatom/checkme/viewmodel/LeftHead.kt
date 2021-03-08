package com.viatom.checkme.viewmodel

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel

class LeftHead : ViewModel() {
    val headIcon: MutableLiveData<Int> by lazy {
        MutableLiveData<Int>()
    }

    val headName: MutableLiveData<String> by lazy {
        MutableLiveData<String>()
    }


}