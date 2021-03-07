package com.viatom.checkme.leftnavi.dailyCheck

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ProgressBar
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.leftnavi.UiChannel
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.ble.format.DlcFile
import com.viatom.checkme.ble.worker.BleDataWorker
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.*
import java.io.File


class DailyCheckFragment : Fragment() {

    private val model: DailyCheckViewModel by viewModels()
    lateinit var dailyViewAdapter: DailyViewAdapter

    @ExperimentalUnsignedTypes
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val root = inflater.inflate(R.layout.fragment_dailycheck, container, false)
        val r: RecyclerView = root.findViewById(R.id.daily_list)
        val pro: ProgressBar = root.findViewById(R.id.pro)
        val wave:RecyclerView=root.findViewById(R.id.ecg_list)


        val linearLayoutManagerWave = LinearLayoutManager(context)
        linearLayoutManagerWave.orientation = RecyclerView.VERTICAL
        wave.layoutManager=linearLayoutManagerWave
        val waveAdapter=WaveAdapter(requireContext(),wave)
        wave.adapter=waveAdapter

        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL

        r.layoutManager = linearLayoutManager
        dailyViewAdapter = DailyViewAdapter(requireContext(), r,wave,waveAdapter)
        r.adapter = dailyViewAdapter





        model.done.observe(viewLifecycleOwner, {
            if (it) {
                pro.visibility = View.VISIBLE
            } else {
                pro.visibility = View.GONE
            }
        })

        model.list.observe(viewLifecycleOwner, {
            dailyViewAdapter.addAll(it)
        })

        model.progress.observe(viewLifecycleOwner, {
            pro.progress = it
        })

        model.waveVisible.observe(viewLifecycleOwner,{
            wave.visibility=if(it){
                View.VISIBLE
            }else{
                View.GONE
            }
        })

        switch(MainActivity.currentId)
        MainScope().launch {

            if (MainActivity.loading) {
                for (k in UiChannel.progressChannel) {
                    model.progress.value = k
                }
            }
            model.done.value = false

            BleDataWorker.fileProgressChannel.receive()

            for (k in BleDataWorker.fileProgressChannel) {
                model.progress.value = k.progress
                model.done.value = true
                if(k.progress==100){
                    delay(300)
                    break
                }
            }
            model.done.value = false


        }
        return root
    }


    @ExperimentalUnsignedTypes
    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "dlc.dat"))
        if (file.exists()) {
            val temp = file.readBytes()
            temp.let {
                val f = DlcFile.DlcInfo(it)
                model.list.value = f.dlc
            }
        } else {
            model.list.value = arrayListOf()
        }
    }
}