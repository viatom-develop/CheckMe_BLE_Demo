package com.viatom.checkme.leftnavi.pulseOximeter

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.EcgBean
import com.viatom.checkme.bean.OxyBean

class PulseOximiterViewModel : ViewModel() {

    val list:MutableLiveData<ArrayList<OxyBean>> by lazy{
        MutableLiveData<ArrayList<OxyBean>>()
    }
}