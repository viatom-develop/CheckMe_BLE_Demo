package com.viatom.checkme.leftnavi.ecgRecorder

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.ProgressBar
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.ble.format.EcgInfo
import com.viatom.checkme.ble.worker.BleDataWorker
import com.viatom.checkme.leftnavi.wave.WaveAdapter
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.io.File

class EcgRecorderFragment : Fragment() {

    private val model: EcgRecorderViewModel by viewModels()
    lateinit var ecgViewAdapter: EcgViewAdapter


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val root = inflater.inflate(R.layout.fragment_ecg, container, false)
        val r: RecyclerView = root.findViewById(R.id.ecglist)
        val linearLayoutManager = LinearLayoutManager(context)
        val pro: ProgressBar = root.findViewById(R.id.pro)
        val wave: RecyclerView = root.findViewById(R.id.ecg_list)
        val waveLayout: LinearLayout = root.findViewById(R.id.wavelayout)
        val hr: TextView = root.findViewById(R.id.p1)
        val st: TextView = root.findViewById(R.id.p2)
        val qrs: TextView = root.findViewById(R.id.p3)
        val pvcs: TextView = root.findViewById(R.id.p4)
        val qtc: TextView = root.findViewById(R.id.p5)
        val qt: TextView = root.findViewById(R.id.p6)
        val linearLayoutManagerWave = LinearLayoutManager(context)
        val backButton: ImageView = root.findViewById(R.id.back_button)



        linearLayoutManagerWave.orientation = RecyclerView.VERTICAL
        wave.layoutManager = linearLayoutManagerWave
        val waveAdapter = WaveAdapter(requireContext(), wave)
        wave.adapter = waveAdapter

        linearLayoutManager.orientation = RecyclerView.VERTICAL
        ecgViewAdapter = EcgViewAdapter(
            requireContext(), r, wave,
            waveAdapter,
            waveLayout,
            model
        )
        r.layoutManager = linearLayoutManager
        r.adapter = ecgViewAdapter


        backButton.setOnClickListener {
            waveLayout.visibility = View.GONE
            r.visibility = View.VISIBLE
            ecgViewAdapter.notifyDataSetChanged()

        }

        model.list.observe(viewLifecycleOwner, {
            ecgViewAdapter.addAll(it)
        })


        model.done.observe(viewLifecycleOwner, {
            if (it) {
                pro.visibility = View.VISIBLE
            } else {
                pro.visibility = View.GONE
            }
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
            qrs.text = "QRS: " + it.qrs.toString() + " ms"
            pvcs.text = "PVCS: " + it.pvcs.toString()
            qtc.text = "QTC: " + it.qtc.toString() + " ms"
            qt.text = "QT: " + it.qt.toString() + " ms"

        })
        switch(MainActivity.currentId)


        MainScope().launch {
            BleDataWorker.fileProgressChannel.receive()
            for (k in BleDataWorker.fileProgressChannel) {
                model.progress.value = k.progress
                model.done.value = true
                if (k.progress == -100) {
                    delay(300)
                    model.done.value = false
                } else {
                    if (model.done.value == false)
                        model.done.value = true
                }
            }
        }
        return root
    }


    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "ecg.dat"))
        if (file.exists()) {
            val temp = file.readBytes()

            temp.let {
                val f = EcgInfo(it)
                model.list.value = f.ecg
            }

        } else {
            model.list.value = arrayListOf()
        }
    }
}