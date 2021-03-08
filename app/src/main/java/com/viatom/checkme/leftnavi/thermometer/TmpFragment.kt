package com.viatom.checkme.leftnavi.thermometer

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
import com.viatom.checkme.ble.format.TmpInfo
import com.viatom.checkme.utils.Constant
import java.io.File


class TmpFragment : Fragment() {

    private val model: TmpViewModel by viewModels()
    lateinit var tmpViewAdapter: TmpViewAdapter

    @ExperimentalUnsignedTypes
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val root = inflater.inflate(R.layout.fragment_thermometer, container, false)
        val r: RecyclerView = root.findViewById(R.id.sleeplist)
        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        tmpViewAdapter = TmpViewAdapter(requireContext())
        r.layoutManager = linearLayoutManager
        r.adapter = tmpViewAdapter
        model.list.observe(viewLifecycleOwner, {
            tmpViewAdapter.addAll(it)
        })
        switch(MainActivity.currentId)
        return root
    }


    @ExperimentalUnsignedTypes
    fun switch(s: String) {
        val file = File(Constant.getPathX(s + "tmp.dat"))
        if (file.exists()) {
            val temp = file.readBytes()

            temp.let {
                val f = TmpInfo(it)
                model.list.value = f.Tmp
            }

        } else {
            model.list.value = arrayListOf()
        }
    }
}