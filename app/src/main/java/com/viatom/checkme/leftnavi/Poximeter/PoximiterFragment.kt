package com.viatom.checkme.leftnavi.Poximeter

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.lifecycle.Observer
import androidx.lifecycle.ViewModelProvider
import com.viatom.checkme.R

class PoximiterFragment : Fragment() {

    private lateinit var poximiterViewModel: PoximiterViewModel

    override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
    ): View? {
        poximiterViewModel =
                ViewModelProvider(this).get(PoximiterViewModel::class.java)
        val root = inflater.inflate(R.layout.fragment_pulseoximeter, container, false)
        val textView: TextView = root.findViewById(R.id.text_slideshow)
        poximiterViewModel.text.observe(viewLifecycleOwner, Observer {
            textView.text = it
        })
        return root
    }
}