package com.viatom.checkme.adapter

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
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.utils.Constant
import java.util.*

class UserViewAdapter(context: Context, r: RecyclerView) :
    RecyclerView.Adapter<UserViewAdapter.ViewHolder>() {
    var mUserData: MutableList<UserBean> = ArrayList()
    private val mInflater: LayoutInflater = LayoutInflater.from(context)
    private var mClickListener: userClickListener? = null
    private val mContext: Context
    private val recyclerView: RecyclerView

    // inflates the cell layout from xml when needed
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = mInflater.inflate(R.layout.item_user, parent, false)
        return ViewHolder(view)
    }

    // binds the data to the TextView in each cell
    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        mUserData[position].apply {
            holder.bleName.text = name
            holder.head.setImageResource(Constant.ICO_IMG[ico - 1])
            if(color==0){
                holder.gaga.setBackgroundColor(Color.parseColor("#FF00FF"))
            }else{
                holder.gaga.setBackgroundColor(Color.parseColor("#0000F0"))
            }

        }

    }



    fun addUser(userBean: UserBean) {
        mUserData.add(userBean)
    }

    fun setUserColor(i:Int){
        for((j,m) in mUserData.withIndex()){
            if(j!=i){
                m.color=1
            }else{
                m.color=0
            }
        }
        notifyDataSetChanged()
    }

    // total number of cells
    override fun getItemCount(): Int {
        return mUserData.size
    }

    inner class ViewHolder internal constructor(itemView: View) : RecyclerView.ViewHolder(itemView),
        View.OnClickListener {
        var bleName: TextView = itemView.findViewById(R.id.userName)
        var gaga: LinearLayout = itemView.findViewById(R.id.gaga)
        var head: ImageView = itemView.findViewById(R.id.head)
        override fun onClick(view: View) {
            setUserColor(layoutPosition)
            mClickListener?.apply {
                onUserItemClick(mUserData[layoutPosition],layoutPosition)
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
        recyclerView = r
        mContext = context
    }
}