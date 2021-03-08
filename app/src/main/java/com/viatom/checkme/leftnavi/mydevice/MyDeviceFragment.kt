package com.viatom.checkme.leftnavi.mydevice

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.leftnavi.dailyCheck.DailyCheckViewModel
import com.viatom.checkme.leftnavi.dailyCheck.DailyViewAdapter
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch


class MyDeviceFragment : Fragment() {

    private val model: DailyCheckViewModel by viewModels()
    lateinit var dailyViewAdapter: DailyViewAdapter
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val root = inflater.inflate(R.layout.fragment_mydevice, container, false)
        val text: TextView = root.findViewById(R.id.text)
        MainScope().launch {
            if (!MainActivity.isOffline) {
                val info = MainActivity.bleWorker.getDeviceInfo()
                val json = info.json
                var s: String = ""
                for (k in json.keys()) {
                    s += "$k: "
                    s += "${json.get(k)} \n"
                }
                text.text = s
            }


        }

        return root
    }


}