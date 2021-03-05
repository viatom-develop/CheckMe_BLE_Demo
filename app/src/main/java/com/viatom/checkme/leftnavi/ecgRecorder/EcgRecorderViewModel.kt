package com.viatom.checkme.leftnavi.ecgRecorder

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.EcgBean

class EcgRecorderViewModel : ViewModel() {

    val list: MutableLiveData<ArrayList<EcgBean>> by lazy {
        MutableLiveData<ArrayList<EcgBean>>()
    }
}