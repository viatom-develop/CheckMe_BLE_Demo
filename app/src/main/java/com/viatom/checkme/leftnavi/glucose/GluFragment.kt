package com.viatom.checkme.leftnavi.glucose

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
import com.viatom.checkme.ble.format.GluInfo
import com.viatom.checkme.ble.format.TmpInfo
import com.viatom.checkme.utils.Constant
import java.io.File


class GluFragment : Fragment() {

    private val model: GluViewModel by viewModels()
    lateinit var gluViewAdapter: GluViewAdapter


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val root = inflater.inflate(R.layout.fragment_glu, container, false)
        val r: RecyclerView = root.findViewById(R.id.sleeplist)
        val linearLayoutManager = LinearLayoutManager(context)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        gluViewAdapter = GluViewAdapter(requireContext())
        r.layoutManager = linearLayoutManager
        r.adapter = gluViewAdapter
        model.list.observe(viewLifecycleOwner, {
            gluViewAdapter.addAll(it)
        })
        switch(MainActivity.currentId)
        return root
    }


    fun switch(s: String) {
        val file = File(Constant.getPathX(s+"glu.dat"))
        if (file.exists()) {
            val temp = file.readBytes()

            temp.let {
                val f = GluInfo(it)
                model.list.value = f.Glu
            }

        } else {
            model.list.value = arrayListOf()
        }
    }
}