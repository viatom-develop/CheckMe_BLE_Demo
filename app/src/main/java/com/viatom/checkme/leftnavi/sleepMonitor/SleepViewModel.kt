package com.viatom.checkme.leftnavi.sleepMonitor

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.SlpBean
import com.viatom.checkme.bean.TmpBean

class SleepViewModel : ViewModel() {

    val list: MutableLiveData<ArrayList<SlpBean>> by lazy {
        MutableLiveData<ArrayList<SlpBean>>()
    }
}