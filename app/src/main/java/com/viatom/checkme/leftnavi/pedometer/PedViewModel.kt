package com.viatom.checkme.leftnavi.pedometer

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.PedBean

class PedViewModel : ViewModel() {

    val list: MutableLiveData<ArrayList<PedBean>> by lazy {
        MutableLiveData<ArrayList<PedBean>>()
    }
}