package com.viatom.checkme.leftnavi.bp

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.lifecycle.MutableLiveData
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.bean.BpBean
import com.viatom.checkme.ble.format.BpInfo
import com.viatom.checkme.ble.format.PedInfo
import com.viatom.checkme.leftnavi.pedometer.PedViewAdapter
import com.viatom.checkme.leftnavi.pedometer.PedViewModel
import com.viatom.checkme.utils.Constant
import java.io.File


class BpFragment : Fragment() {

    lateinit var bpViewAdapter: BpViewAdapter
    val bpList=MutableLiveData<ArrayList<BpBean>>()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val root = inflater.inflate(R.layout.fragment_bp, container, false)
        val r: RecyclerView = root.findViewById(R.id.bplist)
        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        r.layoutManager=linearLayoutManager
        bpViewAdapter= BpViewAdapter(requireContext())
        r.adapter=bpViewAdapter
        bpList.observe(viewLifecycleOwner,{
            bpViewAdapter.addAll(it)
        })
        switch(MainActivity.currentId)
        return root
    }



    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "nibp.dat"))
        if (file.exists()) {
            val temp = file.readBytes()
            temp.let {
                val f = BpInfo(it)
                bpList.value = f.Bp
            }
        } else {
            bpList.value = arrayListOf()
        }
    }

}