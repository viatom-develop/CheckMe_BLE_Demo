package com.viatom.checkme.activity

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.provider.Settings
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.viatom.checkme.R

class PermissionActivity :AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_permission)
        requestPermission()
    }

    private val permissionRequestCode = 521
    private fun checkP(p: String): Boolean {
        return ContextCompat.checkSelfPermission(this, p) == PackageManager.PERMISSION_GRANTED
    }

    private fun requestPermission() {
        val ps: Array<String> = arrayOf(
            Manifest.permission.ACCESS_FINE_LOCATION
        )

        if (!checkP(Manifest.permission.ACCESS_FINE_LOCATION)) {
            ActivityCompat.requestPermissions(this, ps, permissionRequestCode)
            return
        } else {
            initA()
        }

    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        when (requestCode) {
            permissionRequestCode -> if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initA()
            } else {
                startActivity(Intent(Permission@this, MainActivity::class.java))
                this.finish()
            }
            else -> {
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }

    fun initA(){
        if (!isLocationEnabled()) {
            val intent = Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS)
            startActivityForResult(intent, REQUEST_LOCATION)
        } else {
          initB()
        }
    }
    fun initB() {
        val bluetoothManager =
            getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
       val bluetoothAdapter = bluetoothManager.adapter
        if (bluetoothAdapter == null) {
            val enableBtIntent = Intent(
                BluetoothAdapter.ACTION_REQUEST_ENABLE
            )
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT)
            return;
        }
        if (!(bluetoothAdapter!!.isEnabled)) {
            val enableBtIntent = Intent(
                BluetoothAdapter.ACTION_REQUEST_ENABLE
            )
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT)
            return;
        }
        startActivity(Intent(Permission@this, MainActivity::class.java))
        this.finish()
    }
    fun isLocationEnabled(): Boolean {
        var locationMode = 0
        var locationProviders: String
        locationMode = try {
            Settings.Secure.getInt(getContentResolver(), Settings.Secure.LOCATION_MODE)
        } catch (e: Settings.SettingNotFoundException) {
            e.printStackTrace()
            return false
        }
        return locationMode != Settings.Secure.LOCATION_MODE_OFF
    }

    private val REQUEST_LOCATION = 223
    private val REQUEST_ENABLE_BT = 224
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == REQUEST_LOCATION) {
            initB()
        } else if (requestCode == REQUEST_ENABLE_BT) {
            initB()
        }
    }
}