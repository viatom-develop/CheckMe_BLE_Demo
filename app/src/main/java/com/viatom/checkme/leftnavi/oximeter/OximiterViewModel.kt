package com.viatom.checkme.leftnavi.oximeter

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.OxyBean

class OximiterViewModel : ViewModel() {

    val list: MutableLiveData<ArrayList<OxyBean>> by lazy {
        MutableLiveData<ArrayList<OxyBean>>()
    }
}