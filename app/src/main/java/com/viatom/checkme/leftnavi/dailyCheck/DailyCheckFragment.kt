package com.viatom.checkme.leftnavi.dailyCheck

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.View.GONE
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.TextView
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.ble.format.DlcInfo

import com.viatom.checkme.ble.worker.BleDataWorker
import com.viatom.checkme.leftnavi.UiChannel
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.io.File


class DailyCheckFragment : Fragment() {

    private val model: DailyCheckViewModel by viewModels()
    lateinit var dailyViewAdapter: DailyViewAdapter


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val root = inflater.inflate(R.layout.fragment_dailycheck, container, false)
        val r: RecyclerView = root.findViewById(R.id.daily_list)
        val pro: ProgressBar = root.findViewById(R.id.pro)
        val wave: RecyclerView = root.findViewById(R.id.ecg_list)
        val waveLayout: LinearLayout = root.findViewById(R.id.wavelayout)
        val hr: TextView = root.findViewById(R.id.p1)
        val st: TextView = root.findViewById(R.id.p2)
        val qrs: TextView = root.findViewById(R.id.p3)
        val pvcs: TextView = root.findViewById(R.id.p4)
        val qtc: TextView = root.findViewById(R.id.p5)
        val qt: TextView = root.findViewById(R.id.p6)
        val ecgButton: TextView = root.findViewById(R.id.ecg_button)
        val o2Button: TextView = root.findViewById(R.id.o2button)
        val backButton: ImageView = root.findViewById(R.id.back_button)

        backButton.setOnClickListener {
            waveLayout.visibility = GONE
            r.visibility = View.VISIBLE
        }

        ecgButton.setOnClickListener {
            ecgButton.setTextColor(ContextCompat.getColor(requireContext(), R.color.white))
            o2Button.setTextColor(ContextCompat.getColor(requireContext(), R.color.black))
            ecgButton.background = ContextCompat.getDrawable(
                requireContext(),
                R.drawable.button_circle_shape
            )
            o2Button.background = ContextCompat.getDrawable(
                requireContext(),
                R.drawable.button_circle_shape_white
            )
        }
        o2Button.setOnClickListener {
            ecgButton.setTextColor(ContextCompat.getColor(requireContext(), R.color.black))
            o2Button.setTextColor(ContextCompat.getColor(requireContext(), R.color.white))
            ecgButton.background = ContextCompat.getDrawable(
                requireContext(),
                R.drawable.button_circle_shape_white
            )
            o2Button.background = ContextCompat.getDrawable(
                requireContext(),
                R.drawable.button_circle_shape
            )
        }

        ecgButton.callOnClick()

        val linearLayoutManagerWave = LinearLayoutManager(context)
        linearLayoutManagerWave.orientation = RecyclerView.VERTICAL
        wave.layoutManager = linearLayoutManagerWave
        val waveAdapter = WaveAdapter(requireContext(), wave)
        wave.adapter = waveAdapter

        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL

        r.layoutManager = linearLayoutManager
        dailyViewAdapter = DailyViewAdapter(
            requireContext(),
            r,
            wave,
            waveAdapter,
            waveLayout,
            model
        )
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

        model.waveVisible.observe(viewLifecycleOwner, {
            wave.visibility = if (it) {
                View.VISIBLE
            } else {
                View.GONE
            }
        })


        model.waveResult.observe(viewLifecycleOwner, {
            hr.text = "HR: " + it.hr.toString() + " bpm"
            st.text = "ST: " + it.st.toString() + " mV"
            qrs.text = "QRS: " + it.hr.toString() + " ms"
            pvcs.text = "PVCS: " + it.hr.toString()
            qtc.text = "QTC: " + it.hr.toString() + " ms"
            qt.text = "QT: " + it.hr.toString() + " ms"

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
                if (k.progress == 100) {
                    delay(300)
                    break
                }
            }
            model.done.value = false


        }
        return root
    }


    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "dlc.dat"))
        if (file.exists()) {
            val temp = file.readBytes()
            temp.let {
                val f = DlcInfo(it)
                model.list.value = f.dlc
            }
        } else {
            model.list.value = arrayListOf()
        }
    }
}