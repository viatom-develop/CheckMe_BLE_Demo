package com.viatom.checkme.leftnavi.ecgRecorder

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
import com.viatom.checkme.ble.format.EcgFile
import com.viatom.checkme.utils.Constant
import java.io.File

class EcgRecorderFragment : Fragment() {

    private val model: EcgRecorderViewModel by viewModels()
    lateinit var ecgViewAdapter: EcgViewAdapter
    @ExperimentalUnsignedTypes
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val root = inflater.inflate(R.layout.fragment_ecg, container, false)
        val r: RecyclerView = root.findViewById(R.id.ecglist)
        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        ecgViewAdapter = EcgViewAdapter(requireContext(), r)
        r.layoutManager = linearLayoutManager
        r.adapter = ecgViewAdapter
        model.list.observe(viewLifecycleOwner, {
            ecgViewAdapter.addAll(it)
        })
        switch(MainActivity.currentId)
        return root
    }



    @ExperimentalUnsignedTypes
    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "ecg.dat"))
        if (file.exists()) {
            val temp = file.readBytes()

            temp.let {
                val f = EcgFile.EcgInfo(it)
                model.list.value = f.ecg
            }

        } else {
            model.list.value = arrayListOf()
        }
    }
}