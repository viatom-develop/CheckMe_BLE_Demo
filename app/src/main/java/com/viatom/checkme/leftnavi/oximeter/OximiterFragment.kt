package com.viatom.checkme.leftnavi.oximeter

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
import com.viatom.checkme.ble.format.OxyInfo
import com.viatom.checkme.utils.Constant
import java.io.File

class OximiterFragment : Fragment() {
    private val model: OximiterViewModel by viewModels()
    lateinit var oxyViewAdapter: OxyViewAdapter


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val root = inflater.inflate(R.layout.fragment_pulseoximeter, container, false)
        val r: RecyclerView = root.findViewById(R.id.oxylist)
        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        oxyViewAdapter = OxyViewAdapter(requireContext(), r)
        r.layoutManager = linearLayoutManager
        r.adapter = oxyViewAdapter
        model.list.observe(viewLifecycleOwner, {
            oxyViewAdapter.addAll(it)
        })
        switch(MainActivity.currentId)
        return root
    }


    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "oxi.dat"))
        if (file.exists()) {
            val temp = file.readBytes()
            if (!temp.isEmpty()) {
                temp.let {
                    val f = OxyInfo(it)
                    model.list.value = f.Oxy
                }
            }
        } else {
            model.list.value = arrayListOf()
        }
    }
}