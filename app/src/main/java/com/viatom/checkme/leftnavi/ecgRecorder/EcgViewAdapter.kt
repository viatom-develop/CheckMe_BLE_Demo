package com.viatom.checkme.leftnavi.ecgRecorder

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
import com.viatom.checkme.bean.EcgBean
import com.viatom.checkme.ble.format.EcgWaveInfo
import com.viatom.checkme.ble.worker.BleDataWorker
import com.viatom.checkme.leftnavi.wave.WaveAdapter
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

class EcgViewAdapter(
    context: Context,
    val r: RecyclerView,
    val wave: RecyclerView,
    val waveAdapter: WaveAdapter,
    val waveLayout: LinearLayout,
    val model: EcgRecorderViewModel
) :
    RecyclerView.Adapter<EcgViewAdapter.ViewHolder>() {
    var mEcgData: MutableList<EcgBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)


    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_ecg, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {

        mEcgData[position].apply {
            val dateFormat = SimpleDateFormat("MMM. d, yyyy   hh : mm : ss", Locale.ENGLISH)
            holder.bleName.text = dateFormat.format(date)
            holder.face.setImageResource(Constant.RESULT_IMG[face])
            holder.way.text = Constant.EcgWay[way - 1]
            val file = File(Constant.getPathX(timeString))
            holder.download.visibility = if (file.exists()) {
                View.INVISIBLE
            } else {
                View.VISIBLE
            }
        }
    }

    fun add(userBean: EcgBean) {
        mEcgData.add(userBean)
        notifyDataSetChanged()
    }

    fun addAll(userBean: ArrayList<*>?) {
        mEcgData.clear()
        if (userBean != null) {
            for (k in userBean) {
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
        val bleName: TextView = itemView.findViewById(R.id.getTime)
        val way: TextView = itemView.findViewById(R.id.way)
        val face: ImageView = itemView.findViewById(R.id.head)
        val download: ImageView = itemView.findViewById(R.id.download)
        override fun onClick(view: View) {
            val file = File(Constant.getPathX(mEcgData[adapterPosition].timeString))
            val exist = file.exists()
            MainScope().launch {
                if (!exist) {
                    if (!MainActivity.isOffline) {
                        MainActivity.bleWorker.getFile(mEcgData[adapterPosition].timeString)
                    }
                }
                val file2 = File(Constant.getPathX(mEcgData[adapterPosition].timeString))
                BleDataWorker.fileProgressChannel.send(BleDataWorker.FileProgress(progress = -100))
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


}