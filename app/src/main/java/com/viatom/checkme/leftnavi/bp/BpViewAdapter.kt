package com.viatom.checkme.leftnavi.bp

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.bean.BpBean
import com.viatom.checkme.bean.UserBean
import java.text.SimpleDateFormat
import java.util.*

class BpViewAdapter(context: Context) :
    RecyclerView.Adapter<BpViewAdapter.ViewHolder>() {
    var mBpData: MutableList<BpBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: UserClickListener? = null

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_bp, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mBpData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy   hh : mm : ss", Locale.ENGLISH)
            holder.let {
                it.recordTime.text = dateFormat.format(date)
                it.pr.text="pulseRate: $pr"
                it.dia.text="dia: $dia"
                it.sys.text="sys: $sys"
            }

        }
    }

    fun add(userBean: BpBean) {
        mBpData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mBpData.clear()
        if (userBean != null) {
            for (m in userBean) {
                mBpData.add(0,m as BpBean)
            }
        }
        notifyDataSetChanged()
    }


    // total number of cells
    override fun getItemCount(): Int {
        return mBpData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        val recordTime: TextView = itemView.findViewById(R.id.recordTime)
        val sys: TextView = itemView.findViewById(R.id.sys)
        val dia: TextView = itemView.findViewById(R.id.dia)
        val pr: TextView = itemView.findViewById(R.id.pr)
        override fun onClick(view: View) {}

        init {
            itemView.setOnClickListener(this)
        }
    }

    // allows clicks events to be caught
    fun setClickListener(UserClickListener: UserClickListener?) {
        mClickListener = UserClickListener
    }

    interface UserClickListener {
        fun onUserItemClick(userBean: UserBean?, position: Int)
    }

}