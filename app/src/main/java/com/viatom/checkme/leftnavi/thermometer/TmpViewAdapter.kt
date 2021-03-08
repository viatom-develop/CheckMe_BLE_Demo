package com.viatom.checkme.leftnavi.thermometer

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.bean.TmpBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.utils.Constant
import java.text.SimpleDateFormat
import java.util.*

class TmpViewAdapter(context: Context) :
    RecyclerView.Adapter<TmpViewAdapter.ViewHolder>() {
    var mTmpData: MutableList<TmpBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: userClickListener? = null

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_tmp, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mTmpData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy | hh : mm : ss", Locale.ENGLISH)
            holder.time.text = dateFormat.format(date)
            holder.tmp.text = Constant.TmpWay[way] + ":  " + tmp.toString() + " ℃"
            holder.face.setImageResource(Constant.RESULT_IMG[face])
        }
    }

    fun add(userBean: TmpBean) {
        mTmpData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mTmpData.clear()
        if (userBean != null) {
            for (m in userBean) {
                mTmpData.add(m as TmpBean)
            }
        }
        notifyDataSetChanged()
    }


    // total number of cells
    override fun getItemCount(): Int {
        return mTmpData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        val tmp: TextView = itemView.findViewById(R.id.tmp)
        val time: TextView = itemView.findViewById(R.id.time)
        val face: ImageView = itemView.findViewById(R.id.face)
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

    }
}