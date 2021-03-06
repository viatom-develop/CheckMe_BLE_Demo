package com.viatom.checkme.leftnavi.ecgRecorder

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.bean.EcgBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.utils.Constant
import java.text.SimpleDateFormat
import java.util.*

class EcgViewAdapter(context: Context, r: RecyclerView) :
    RecyclerView.Adapter<EcgViewAdapter.ViewHolder>() {
    var mEcgData: MutableList<EcgBean>
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: userClickListener? = null
    private val mContext: Context
    private val recyclerView: RecyclerView

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_ecg, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {

        mEcgData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy | hh : mm : ss", Locale.ENGLISH)
            holder.bleName.text =dateFormat.format(date)
            holder.face.setImageResource(Constant.RESULT_IMG[face])
            holder.way.text=Constant.EcgWay[way-1]
        }
    }

    fun add(userBean: EcgBean) {
        mEcgData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mEcgData.clear()
        if (userBean != null) {
            for(k in userBean){
                mEcgData.add(k as EcgBean)
            }
        }

        notifyDataSetChanged()
    }

    // total number of cells
    override fun getItemCount(): Int {
        return mEcgData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        val bleName: TextView = itemView.findViewById(R.id.userName)
        val way: TextView=itemView.findViewById(R.id.way)
        val face:ImageView=itemView.findViewById(R.id.head)
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
        mEcgData = ArrayList()
        recyclerView = r
        mContext = context
    }
}