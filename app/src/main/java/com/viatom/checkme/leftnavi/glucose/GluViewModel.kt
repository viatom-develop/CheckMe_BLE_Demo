package com.viatom.checkme.leftnavi.glucose

import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.GluBean
import com.viatom.checkme.bean.TmpBean

class GluViewModel : ViewModel() {

    val list: MutableLiveData<ArrayList<GluBean>> by lazy {
        MutableLiveData<ArrayList<GluBean>>()
    }
}