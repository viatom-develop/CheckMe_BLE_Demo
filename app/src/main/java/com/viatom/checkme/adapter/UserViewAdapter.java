package com.viatom.checkme.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.viatom.checkme.R;
import com.viatom.checkme.bean.UserBean;

import java.util.ArrayList;
import java.util.List;

public class UserViewAdapter extends RecyclerView.Adapter<UserViewAdapter.ViewHolder> {
    private List<UserBean> mUserData;
    private LayoutInflater mInflater;
    private userClickListener mClickListener;
    private Context mContext;

    // data is passed into the constructor
    public UserViewAdapter(Context context) {
        this.mInflater = LayoutInflater.from(context);
        this.mUserData = new ArrayList<>();
        mContext = context;
    }




    // inflates the cell layout from xml when needed
    @Override
    @NonNull
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = mInflater.inflate(R.layout.ble_scan_view, parent, false);
        return new ViewHolder(view);
    }

    // binds the data to the TextView in each cell
    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        holder.bleName.setText(mUserData.get(position).getName());

    }

    public void addUser(UserBean userBean) {
        mUserData.add(userBean);
        notifyDataSetChanged();
    }





    // total number of cells
    @Override
    public int getItemCount() {
        return mUserData.size();
    }


    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        TextView bleName;
        ViewHolder(View itemView) {
            super(itemView);
            bleName=itemView.findViewById(R.id.ble_name);
            itemView.setOnClickListener(this);
        }
        @Override
        public void onClick(View view) {
            if (mClickListener != null) mClickListener.onScanItemClick(mUserData.get(getAdapterPosition()));
        }
    }



    // allows clicks events to be caught
    public void setClickListener(userClickListener userClickListener) {
        this.mClickListener = userClickListener;
    }

    public interface userClickListener {
        void onScanItemClick(UserBean userBean);
    }
}