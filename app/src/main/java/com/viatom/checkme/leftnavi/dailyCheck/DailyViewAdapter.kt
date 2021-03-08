package com.viatom.checkme.leftnavi.dailyCheck

import android.content.Context
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
import com.viatom.checkme.ble.format.EcgWaveInfo
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

class DailyViewAdapter(
    context: Context,
    var r: RecyclerView,
    val wave: RecyclerView,
    val waveAdapter: WaveAdapter,
    val waveLayout: LinearLayout,
    val model: DailyCheckViewModel
) :
    RecyclerView.Adapter<DailyViewAdapter.ViewHolder>() {
    var mDlcData: MutableList<DlcBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: userClickListener? = null


    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_daily, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mDlcData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy \n hh : mm : ss", Locale.ENGLISH)
            holder.bleName.text = dateFormat.format(date)
            holder.ecgText.text = hr.toString()
            holder.bpi.text = pr.toString()
            holder.pi.text = pi.toString()
            holder.o2.text = oxy.toString()
            holder.f1.setImageResource(Constant.RESULT_IMG[eface])
            holder.f2.setImageResource(Constant.RESULT_IMG[oface])
            holder.f3.setImageResource(Constant.RESULT_IMG[bpiFace])
            val file = File(Constant.getPathX(timeString))
            holder.download.visibility = if (file.exists()) {
                View.INVISIBLE
            } else {
                View.VISIBLE
            }
        }

    }

    fun add(userBean: DlcBean) {
        mDlcData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mDlcData.clear()
        if (userBean != null) {
            for (k in userBean) {
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
        val ecgText: TextView = itemView.findViewById(R.id.ecg)
        val o2: TextView = itemView.findViewById(R.id.o2)
        val pi: TextView = itemView.findViewById(R.id.pi)
        val bpi: TextView = itemView.findViewById(R.id.bpi)
        val f1: ImageView = itemView.findViewById(R.id.f1)
        val f2: ImageView = itemView.findViewById(R.id.f2)
        val f3: ImageView = itemView.findViewById(R.id.f3)
        val download: ImageView = itemView.findViewById(R.id.download)


        override fun onClick(view: View) {
            val file = File(Constant.getPathX(mDlcData[adapterPosition].timeString))
            val exist = file.exists()
            MainScope().launch {
                if (!exist) {
                    if (!MainActivity.isOffline) {
                        MainActivity.bleWorker.getFile(mDlcData[adapterPosition].timeString)
                    }
                }
                val file2 = File(Constant.getPathX(mDlcData[adapterPosition].timeString))
                if (file2.exists()) {
                    val info = EcgWaveInfo(file2.readBytes())
                    waveAdapter.data = info
                    waveAdapter.notifyDataSetChanged()
                    model.waveResult.value = info
                    waveLayout.visibility = View.VISIBLE
                }

            }
            r.visibility = View.GONE
            if (!exist) {
                if (MainActivity.isOffline) {
                    r.visibility = View.VISIBLE
                }
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


}