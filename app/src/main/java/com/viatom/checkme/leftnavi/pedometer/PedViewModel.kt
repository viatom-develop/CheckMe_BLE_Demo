package com.viatom.checkme.leftnavi.pedometer

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.viatom.checkme.bean.EcgBean
import com.viatom.checkme.bean.OxyBean
import com.viatom.checkme.bean.PedBean

class PedViewModel : ViewModel() {

    val list:MutableLiveData<ArrayList<PedBean>> by lazy{
        MutableLiveData<ArrayList<PedBean>>()
    }
}