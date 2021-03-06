package com.viatom.checkme.ble.manager

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.util.Log


class BleScanManager {
    interface Scan {
        fun scanReturn(name: String, bluetoothDevice: BluetoothDevice)
    }

    private var bluetoothAdapter: BluetoothAdapter? = null
    private lateinit var leScanner: BluetoothLeScanner
    private var scan: Scan? = null
    private val leScanCallback: ScanCallback = object : ScanCallback() {
        override fun onScanResult(
            callbackType: Int,
            result: ScanResult
        ) {
            super.onScanResult(callbackType, result)
            val device = result.device
            if (device?.name == null) return;
            scan?.apply {
                scanReturn(device.name, device)
            }
            Log.i("scanned ble", " ${device.name}")
        }

        override fun onBatchScanResults(results: List<ScanResult>) {}
        override fun onScanFailed(errorCode: Int) {}
    }

    fun setCallBack(scan: Scan) {
        this.scan = scan
    }

    fun initScan(context: Context) {
        context.apply {
            val settings: ScanSettings = ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                .build()


            val bluetoothManager =
                getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
            bluetoothAdapter = bluetoothManager.adapter
            leScanner = bluetoothAdapter!!.bluetoothLeScanner
            leScanner.startScan(null, settings, leScanCallback)
        }
    }

    fun stop() {
        leScanner.stopScan(leScanCallback)
    }
}