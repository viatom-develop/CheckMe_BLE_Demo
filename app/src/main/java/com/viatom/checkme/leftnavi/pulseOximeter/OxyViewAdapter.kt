package com.viatom.checkme.leftnavi.pulseOximeter

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.bean.OxyBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.utils.Constant
import java.text.SimpleDateFormat
import java.util.*

class OxyViewAdapter(context: Context, r: RecyclerView) :
    RecyclerView.Adapter<OxyViewAdapter.ViewHolder>() {
    var mOxyData: MutableList<OxyBean>
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: userClickListener? = null
    private val mContext: Context
    private val recyclerView: RecyclerView

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_oxy, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bleName.text = mOxyData[position].timeString

        mOxyData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy \n hh : mm : ss", Locale.ENGLISH)
            holder.bleName.text =dateFormat.format(date)

        }
    }

    fun add(userBean: OxyBean) {
        mOxyData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mOxyData.clear()
        if (userBean != null) {
            for(k in userBean){
                mOxyData.add(k as OxyBean)
            }
        }

        notifyDataSetChanged()
    }

    // total number of cells
    override fun getItemCount(): Int {
        return mOxyData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        val bleName: TextView = itemView.findViewById(R.id.userName)

        override fun onClick(view: View) {}

        init {
            itemView.setOnClickListener(this)
        }
    }

    // allows clicks events to be caught
    fun setClickListener(userClickListener: userClickListener?) {
        mClickListener = userClickListener
    }

    interface userClickListener {
        fun onUserItemClick(userBean: UserBean?, position: Int)
    }

    // data is passed into the constructor
    init {
        mOxyData = ArrayList()
        recyclerView = r
        mContext = context
    }
}