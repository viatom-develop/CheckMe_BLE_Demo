package com.viatom.checkme.ui.DCheck

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.lifecycle.Observer
import androidx.lifecycle.ViewModelProvider
import com.viatom.checkme.R


class DcheckFragment : Fragment() {

    private lateinit var dcheckViewModel: DcheckViewModel

    override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
    ): View? {
        dcheckViewModel =
                ViewModelProvider(this).get(DcheckViewModel::class.java)
        val root = inflater.inflate(R.layout.fragment_ecg, container, false)
        val textView: TextView = root.findViewById(R.id.text_gallery)
        dcheckViewModel.text.observe(viewLifecycleOwner, Observer {
            textView.text = it
        })
        return root
    }
}