package com.viatom.checkme.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;


import com.viatom.checkme.R;
import com.viatom.fda.bean.PanelBean;

import java.util.ArrayList;
import java.util.List;

public class BlePanelAdapter extends RecyclerView.Adapter<BlePanelAdapter.ViewHolder> {
    private List<PanelBean> mBlePanelData;
    private LayoutInflater mInflater;
    private ItemClickListener mClickListener;
    private Context mContext;

    // data is passed into the constructor
    public BlePanelAdapter(Context context) {
        this.mInflater = LayoutInflater.from(context);
        this.mBlePanelData = new ArrayList<>();
        mContext = context;
    }




    // inflates the cell layout from xml when needed
    @Override
    @NonNull
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = mInflater.inflate(R.layout.ble_panel_view, parent, false);
        return new ViewHolder(view);
    }

    // binds the data to the TextView in each cell
    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        holder.bleName.setText(mBlePanelData.get(position).getName());

    }

    public void addDevice(String name) {
        mBlePanelData.add(new PanelBean(name));
        notifyDataSetChanged();
    }


    public void addDevice(String[] name) {
        for(int k=0;k<name.length;k++){
            mBlePanelData.add(new PanelBean(name[k]));
        }
        notifyDataSetChanged();
    }


    // total number of cells
    @Override
    public int getItemCount() {
        return mBlePanelData.size();
    }


    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        TextView bleName;
        ViewHolder(View itemView) {
            super(itemView);
            bleName=itemView.findViewById(R.id.ble_name);
            itemView.setOnClickListener(this);
        }




        @Override
        public void onClick(View v) {
            if (mClickListener != null) mClickListener.onPanelClick(getAdapterPosition());
        }
    }



    // allows clicks events to be caught
    public void setClickListener(ItemClickListener itemClickListener) {
        this.mClickListener = itemClickListener;
    }

    public interface ItemClickListener {
        void onPanelClick(int index);
    }
}