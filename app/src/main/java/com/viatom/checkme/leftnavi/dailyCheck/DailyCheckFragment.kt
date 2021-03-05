package com.viatom.checkme.leftnavi.dailyCheck

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
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.ble.format.DlcFile
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import java.io.File


class DailyCheckFragment : Fragment() {

    private val model: DailyCheckViewModel by viewModels()
    lateinit var dailyViewAdapter: DailyViewAdapter
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {

        val root = inflater.inflate(R.layout.fragment_dailycheck, container, false)
        val r:RecyclerView=root.findViewById(R.id.daily_list)
        val pro:ProgressBar=root.findViewById(R.id.pro)

        val linearLayoutManager=LinearLayoutManager(context)
        linearLayoutManager.orientation=RecyclerView.VERTICAL
        dailyViewAdapter= DailyViewAdapter(context,r)
        r.layoutManager=linearLayoutManager
        r.adapter=dailyViewAdapter

        model.done.observe(viewLifecycleOwner,{
            if(it){
                pro.visibility=View.VISIBLE
            }else{
                pro.visibility=View.GONE
            }
        })

        model.list.observe(viewLifecycleOwner,{
            dailyViewAdapter.addAll(it)
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

            model.done.value=false
            val file=File(Constant.getPathX("1dlc.dat"))
            if(file.exists()){
                val temp=file.readBytes()
                temp.let {
                    val f=DlcFile.DlcInfo(it)
                    model.list.value=f.dlc
                }
            }


        }
    }
}