package com.viatom.checkme.leftnavi.ecgRecorder

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.EcgBean
import com.viatom.checkme.ble.format.EcgWaveInfo

class EcgRecorderViewModel : ViewModel() {

    val list: MutableLiveData<ArrayList<EcgBean>> by lazy {
        MutableLiveData<ArrayList<EcgBean>>()
    }


    val text: MutableLiveData<String> by lazy {
        MutableLiveData<String>()
    }

    val done: MutableLiveData<Boolean> by lazy {
        MutableLiveData<Boolean>()
    }


    val progress: MutableLiveData<Int> by lazy {
        MutableLiveData<Int>()
    }

    val waveVisible: MutableLiveData<Boolean> by lazy {
        MutableLiveData<Boolean>()
    }

    val waveResult: MutableLiveData<EcgWaveInfo> by lazy {
        MutableLiveData<EcgWaveInfo>()
    }
}