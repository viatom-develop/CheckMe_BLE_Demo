package com.viatom.checkme.leftnavi.ecgRecorder

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.lifecycle.Observer
import androidx.lifecycle.ViewModelProvider
import com.viatom.checkme.R

class ErecorderFragment : Fragment() {

    private lateinit var erecorderViewModel: ErecorderViewModel

    override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
    ): View? {
        erecorderViewModel =
                ViewModelProvider(this).get(ErecorderViewModel::class.java)
        val root = inflater.inflate(R.layout.fragment_ecg, container, false)


        return root
    }
}