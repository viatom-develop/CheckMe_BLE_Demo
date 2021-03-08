package com.viatom.checkme.leftnavi.sleepMonitor

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
import com.viatom.checkme.ble.format.SlpInfo
import com.viatom.checkme.utils.Constant
import java.io.File


class SleepFragment : Fragment() {

    private val model: SleepViewModel by viewModels()
    lateinit var sleepViewAdapter: SleepViewAdapter


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val root = inflater.inflate(R.layout.fragment_sleepmonitor, container, false)
        val r: RecyclerView = root.findViewById(R.id.sleeplist)
        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        sleepViewAdapter = SleepViewAdapter(requireContext())
        r.layoutManager = linearLayoutManager
        r.adapter = sleepViewAdapter
        model.list.observe(viewLifecycleOwner, {
            sleepViewAdapter.addAll(it)
        })
        switch(MainActivity.currentId)
        return root
    }


    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "tmp.dat"))
        if (file.exists()) {
            val slp = file.readBytes()

            slp.let {
                val f = SlpInfo(it)
                model.list.value = f.Slp
            }

        } else {
            model.list.value = arrayListOf()
        }
    }
}