package com.viatom.checkme.leftnavi.bp

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.bean.PedBean
import com.viatom.checkme.bean.UserBean
import java.text.SimpleDateFormat
import java.util.*

class BpViewAdapter(context: Context) :
    RecyclerView.Adapter<BpViewAdapter.ViewHolder>() {
    var mPedData: MutableList<PedBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: UserClickListener? = null

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_ped, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mPedData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy   hh : mm : ss", Locale.ENGLISH)
            holder.recordTime.text = dateFormat.format(date)
            holder.step.text = step.toString()
            holder.dis.text = dis.toString()
            holder.speed.text = speed.toString()
            holder.cal.text = cal.toString()
            holder.fat.text = fat.toString()
            holder.time.text = time.toString()
        }
    }

    fun add(userBean: PedBean) {
        mPedData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mPedData.clear()
        if (userBean != null) {
            for (m in userBean) {
                mPedData.add(m as PedBean)
            }
        }
        notifyDataSetChanged()
    }


    // total number of cells
    override fun getItemCount(): Int {
        return mPedData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        val recordTime: TextView = itemView.findViewById(R.id.recordTime)
        val step: TextView = itemView.findViewById(R.id.step)
        val dis: TextView = itemView.findViewById(R.id.dis)
        val speed: TextView = itemView.findViewById(R.id.speed)
        val cal: TextView = itemView.findViewById(R.id.cal)
        val fat: TextView = itemView.findViewById(R.id.fat)
        val time: TextView = itemView.findViewById(R.id.time)
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