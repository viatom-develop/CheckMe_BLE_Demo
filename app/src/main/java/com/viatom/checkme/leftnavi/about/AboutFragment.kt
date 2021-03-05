package com.viatom.checkme.leftnavi.about

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ProgressBar
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.Chanl
import com.viatom.checkme.R
import com.viatom.checkme.ble.format.DlcFile
import com.viatom.checkme.leftnavi.dailyCheck.DailyCheckViewModel
import com.viatom.checkme.leftnavi.dailyCheck.DailyViewAdapter
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import java.io.File


class AboutFragment : Fragment() {

    private val model: DailyCheckViewModel by viewModels()
    lateinit var dailyViewAdapter: DailyViewAdapter
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val root = inflater.inflate(R.layout.fragment_about, container, false)


        return root
    }


}