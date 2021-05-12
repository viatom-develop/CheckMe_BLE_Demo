package com.viatom.checkme.leftnavi.bp

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.ble.format.PedInfo
import com.viatom.checkme.leftnavi.pedometer.PedViewAdapter
import com.viatom.checkme.leftnavi.pedometer.PedViewModel
import com.viatom.checkme.utils.Constant
import java.io.File


class BpFragment : Fragment() {


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val root = inflater.inflate(R.layout.fragment_pedometer, container, false)
        val r: RecyclerView = root.findViewById(R.id.pedlist)
        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        pedViewAdapter = PedViewAdapter(requireContext())
        r.layoutManager = linearLayoutManager
        r.adapter = pedViewAdapter
        model.list.observe(viewLifecycleOwner, {
            pedViewAdapter.addAll(it)
        })
        switch(MainActivity.currentId)
        return root
    }


    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "ped.dat"))
        if (file.exists()) {
            val temp = file.readBytes()
            temp.let {
                val f = PedInfo(it)
                f.Ped
            }
        } else {

        }
    }

}