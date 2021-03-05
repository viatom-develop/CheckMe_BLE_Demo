package com.viatom.checkme.leftnavi.about

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import com.viatom.checkme.R
import com.viatom.checkme.leftnavi.dailyCheck.DailyCheckViewModel
import com.viatom.checkme.leftnavi.dailyCheck.DailyViewAdapter


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