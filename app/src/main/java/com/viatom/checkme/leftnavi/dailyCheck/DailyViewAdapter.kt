package com.viatom.checkme.leftnavi.dailyCheck

import android.content.Context
import android.graphics.Color
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.bean.DlcBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.utils.Constant
import kotlinx.android.synthetic.main.right_drawer.*
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

class DailyViewAdapter(context: Context, r: RecyclerView) :
    RecyclerView.Adapter<DailyViewAdapter.ViewHolder>() {
    var mDlcData: MutableList<DlcBean>
    private val mInflater: LayoutInflater
    private var mClickListener: userClickListener? = null
    private val mContext: Context
    private val recyclerView: RecyclerView

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_daily, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mDlcData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy \n hh : mm : ss", Locale.ENGLISH)
            holder.bleName.text =dateFormat.format(date)
            holder.ecgText.text=hr.toString()
            holder.bpi.text=pr.toString()
            holder.pi.text=pi.toString()
            holder.o2.text=oxy.toString()
            holder.f1.setImageResource(Constant.RESULT_IMG[eface])
            holder.f2.setImageResource(Constant.RESULT_IMG[oface])
            holder.f3.setImageResource(Constant.RESULT_IMG[bpiFace])
        }

    }

    fun add(userBean: DlcBean) {
        mDlcData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mDlcData.clear()
        if (userBean != null) {
            for(k in userBean){
                mDlcData.add(k as DlcBean)
            }
        }

        notifyDataSetChanged()
    }


    // total number of cells
    override fun getItemCount(): Int {
        return mDlcData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        val bleName: TextView = itemView.findViewById(R.id.userName)
        val ecgText:TextView=itemView.findViewById(R.id.ecg)
        val o2:TextView=itemView.findViewById(R.id.o2)
        val pi:TextView=itemView.findViewById(R.id.pi)
        val bpi:TextView=itemView.findViewById(R.id.bpi)
        val f1:ImageView=itemView.findViewById(R.id.f1)
        val f2:ImageView=itemView.findViewById(R.id.f2)
        val f3:ImageView=itemView.findViewById(R.id.f3)

        override fun onClick(view: View) {
            MainScope().launch {
                MainActivity.bleWorker.getFile(mDlcData[adapterPosition].timeString)
            }
        }

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
        mInflater = LayoutInflater.from(context)
        mDlcData = ArrayList()
        recyclerView = r
        mContext = context
    }
}