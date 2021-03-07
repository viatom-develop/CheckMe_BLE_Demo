package com.viatom.checkme.leftnavi.dailyCheck

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.DlcBean

class DailyCheckViewModel : ViewModel() {

    val text: MutableLiveData<String> by lazy {
        MutableLiveData<String>()
    }

    val done: MutableLiveData<Boolean> by lazy {
        MutableLiveData<Boolean>()
    }


    val list: MutableLiveData<ArrayList<DlcBean>> by lazy {
        MutableLiveData<ArrayList<DlcBean>>()
    }

    val progress: MutableLiveData<Int> by lazy {
        MutableLiveData<Int>()
    }

    val waveVisible: MutableLiveData<Boolean> by lazy {
        MutableLiveData<Boolean>()
    }
}