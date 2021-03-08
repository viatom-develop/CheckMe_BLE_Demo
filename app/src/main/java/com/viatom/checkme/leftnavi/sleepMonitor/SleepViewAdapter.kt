package com.viatom.checkme.leftnavi.sleepMonitor

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.bean.SlpBean
import com.viatom.checkme.utils.Constant
import java.text.SimpleDateFormat
import java.util.*

class SleepViewAdapter(context: Context) :
    RecyclerView.Adapter<SleepViewAdapter.ViewHolder>() {
    var mSlpData: MutableList<SlpBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_slm, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mSlpData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy   hh : mm : ss", Locale.ENGLISH)
            holder.time.text = dateFormat.format(date)
            holder.face.setImageResource(Constant.RESULT_IMG[face])
            holder.lastTime.text = "Duration: " + time.toString() + " s"
            holder.minO2.text = "Min: " + minO2.toString()
            holder.meanO2.text = "Mean: " + meanO2.toString()
            holder.lowTime.text = "Low time: " + lowTime.toString()
            holder.lowCount.text = "Low count: " + lowCount.toString()
        }
    }

    fun add(userBean: SlpBean) {
        mSlpData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mSlpData.clear()
        if (userBean != null) {
            for (m in userBean) {
                mSlpData.add(m as SlpBean)
            }
        }
        notifyDataSetChanged()
    }


    // total number of cells
    override fun getItemCount(): Int {
        return mSlpData.size
    }

    inner class ViewHolder internal constructor(itemView: View) :
        RecyclerView.ViewHolder(itemView) {

        val time: TextView = itemView.findViewById(R.id.time)
        val face: ImageView = itemView.findViewById(R.id.face)
        val lastTime: TextView = itemView.findViewById(R.id.lastTime)
        val minO2: TextView = itemView.findViewById(R.id.min)
        val meanO2: TextView = itemView.findViewById(R.id.mean)
        val lowTime: TextView = itemView.findViewById(R.id.lowTime)
        val lowCount: TextView = itemView.findViewById(R.id.lowCount)

    }


}