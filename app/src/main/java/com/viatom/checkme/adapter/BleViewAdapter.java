package com.viatom.checkme.adapter;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.viatom.checkme.R;
import com.viatom.checkme.bean.BleBean;

import java.util.ArrayList;
import java.util.List;

public class BleViewAdapter extends RecyclerView.Adapter<BleViewAdapter.ViewHolder> {
    private List<BleBean> mBleData;
    private LayoutInflater mInflater;
    private ItemClickListener mClickListener;
    private Context mContext;

    // data is passed into the constructor
    public BleViewAdapter(Context context) {
        this.mInflater = LayoutInflater.from(context);
        this.mBleData = new ArrayList<>();
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
        holder.bleName.setText(mBleData.get(position).getName());

    }

    public void addDevice(String name, BluetoothDevice bluetoothDevice) {
        mBleData.add(new BleBean(name, bluetoothDevice));
        notifyDataSetChanged();
    }


    // total number of cells
    @Override
    public int getItemCount() {
        return mBleData.size();
    }


    public class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        TextView bleName;

        ViewHolder(View itemView) {
            super(itemView);
            bleName = itemView.findViewById(R.id.ble_name);
            itemView.setOnClickListener(this);
        }

        @Override
        public void onClick(View view) {
            if (mClickListener != null)
                mClickListener.onScanItemClick(mBleData.get(getAdapterPosition()).getBluetoothDevice());
        }
    }


    // allows clicks events to be caught
    public void setClickListener(ItemClickListener itemClickListener) {
        this.mClickListener = itemClickListener;
    }

    public interface ItemClickListener {
        void onScanItemClick(BluetoothDevice bluetoothDevice);
    }
}