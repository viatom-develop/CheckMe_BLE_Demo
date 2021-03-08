package com.viatom.checkme.leftnavi.dailyCheck

import android.content.Context
import android.graphics.Color
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.RelativeLayout
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.viatom.checkme.R
import com.viatom.checkme.activity.MainActivity
import com.viatom.checkme.bean.DlcBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.ble.constant.BTConstant
import com.viatom.checkme.ble.format.EcgWaveFile
import com.viatom.checkme.ble.format.OxyFile
import com.viatom.checkme.utils.Constant
import kotlinx.android.synthetic.main.right_drawer.*
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

class WaveAdapter constructor(val context: Context, var r: RecyclerView) :
    RecyclerView.Adapter<WaveAdapter.ViewHolder>() {
    var mDlcData: MutableList<DlcBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: userClickListener? = null
    @ExperimentalUnsignedTypes
    var data:EcgWaveFile.EcgWaveInfo?=null
    var data2:OxyFile.OxyInfo?=null


    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_wave, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.re.measure(0,0)
        val waveView=WaveView(context)
        waveView.data=data?.getWave(position)
        holder.re.addView(waveView)

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
    @ExperimentalUnsignedTypes
    override fun getItemCount(): Int {
        return data?.waveViewSize ?: 0
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView) {
        val re: RelativeLayout =itemView.findViewById(R.id.kk)
    }

    // allows clicks events to be caught
    fun setClickListener(userClickListener: userClickListener?) {
        mClickListener = userClickListener
    }

    interface userClickListener {
        fun onUserItemClick(userBean: UserBean?, position: Int)
    }


}