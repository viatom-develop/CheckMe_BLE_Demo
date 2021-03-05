package com.viatom.checkme.leftnavi.thermometer

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.TmpBean

class TmpViewModel : ViewModel() {

    val list: MutableLiveData<ArrayList<TmpBean>> by lazy {
        MutableLiveData<ArrayList<TmpBean>>()
    }
}