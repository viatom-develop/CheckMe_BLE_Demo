package com.viatom.checkme.leftnavi.pulseOximeter

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.lifecycle.ViewModelProvider
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.Chanl
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.ble.format.EcgFile
import com.viatom.checkme.ble.format.OxyFile
import com.viatom.checkme.leftnavi.ecgRecorder.EcgRecorderViewModel
import com.viatom.checkme.leftnavi.ecgRecorder.EcgViewAdapter
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import java.io.File

class PulseOximiterFragment : Fragment() {
    private val model: PulseOximiterViewModel by viewModels()
    lateinit var oxyViewAdapter: OxyViewAdapter
    override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
    ): View? {

        val root = inflater.inflate(R.layout.fragment_pulseoximeter, container, false)
        val r: RecyclerView =root.findViewById(R.id.oxylist)
        val linearLayoutManager= LinearLayoutManager(context)
        linearLayoutManager.orientation= RecyclerView.VERTICAL
        oxyViewAdapter= OxyViewAdapter(context,r)
        r.layoutManager=linearLayoutManager
        r.adapter=oxyViewAdapter
        model.list.observe(viewLifecycleOwner,{
           oxyViewAdapter.addAll(it)
        })
        return root
    }
    @ExperimentalUnsignedTypes
    override fun onActivityCreated(savedInstanceState: Bundle?) {
        super.onActivityCreated(savedInstanceState)

        MainScope().launch {
            if(MainActivity.loading){
                Chanl.teChannel.receive()
            }
            val temp= File(Constant.getPathX("1oxi.dat")).readBytes()
            if(!temp.isEmpty()) {
                temp.let {
                    val f = OxyFile.OxyInfo(it)
                    model.list.value = f.Oxy
                }
            }

        }
    }
}