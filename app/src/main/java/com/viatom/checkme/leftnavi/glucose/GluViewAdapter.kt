package com.viatom.checkme.leftnavi.glucose

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.bean.GluBean

import java.text.SimpleDateFormat
import java.util.*

class GluViewAdapter(context: Context) :
    RecyclerView.Adapter<GluViewAdapter.ViewHolder>() {
    var mGluData: MutableList<GluBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_glu, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mGluData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy   hh : mm : ss", Locale.ENGLISH)
            holder.time.text = dateFormat.format(date)
            holder.Glu.text = "Blood glucose :  $glu"
        }
    }

    fun add(userBean: GluBean) {
        mGluData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mGluData.clear()
        if (userBean != null) {
            for (m in userBean) {
                mGluData.add(m as GluBean)
            }
        }
        notifyDataSetChanged()
    }


    // total number of cells
    override fun getItemCount(): Int {
        return mGluData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        val Glu: TextView = itemView.findViewById(R.id.tmp)
        val time: TextView = itemView.findViewById(R.id.time)
        override fun onClick(view: View) {}

        init {
            itemView.setOnClickListener(this)
        }
    }



}