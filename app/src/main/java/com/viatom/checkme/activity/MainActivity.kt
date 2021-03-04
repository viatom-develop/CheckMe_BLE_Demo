package com.viatom.checkme.activity

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.graphics.Color
import android.os.Bundle
import android.os.Vibrator
import android.util.Log
import android.view.View
import android.view.View.GONE
import android.view.View.VISIBLE
import android.view.WindowManager
import android.widget.ImageView
import android.widget.TextView
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.navigation.NavigationView
import com.vaca.x1.UserFile
import com.vaca.x1.utils.add
import com.vaca.x1.utils.toUInt
import com.viatom.checkme.utils.Constant
import com.viatom.checkme.utils.Constant.getPathX
import com.viatom.checkme.ble.FDABleManager
import com.viatom.checkme.R
import com.viatom.checkme.adapter.BleViewAdapter
import com.viatom.checkme.adapter.UserViewAdapter
import com.viatom.checkme.bean.BleBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.ble.EndReadPkg
import com.viatom.checkme.ble.FDAResponse
import com.viatom.checkme.ble.ReadContentPkg
import com.viatom.checkme.ble.StartReadPkg
import com.viatom.checkme.utils.CRCUtils.calCRC8
import com.viatom.checkme.viewmodel.LeftHead
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.android.synthetic.main.right_drawer.*
import kotlinx.android.synthetic.main.scan_view.*
import no.nordicsemi.android.ble.data.Data
import java.io.File
import java.lang.Thread.sleep
import java.text.SimpleDateFormat
import java.util.Locale.ENGLISH
import kotlin.experimental.inv

class MainActivity : AppCompatActivity(), BleViewAdapter.ItemClickListener,
    FDABleManager.onNotifyListener, UserViewAdapter.userClickListener {
    private lateinit var appBarConfiguration: AppBarConfiguration
    private var bluetoothAdapter: BluetoothAdapter? = null
    private lateinit var leScanner: BluetoothLeScanner
    private val bleList: MutableList<BleBean> = ArrayList<BleBean>()
    lateinit var bleViewAdapter: BleViewAdapter
    private var cmdState = 0;

    var currentFileName=""


    var userfileName = arrayOf(
            "dlc.dat",
            "spc.dat",
            "bpcal.dat",
            "ecg.dat",
            "oxi.dat",
            "tmp.dat",
            "slm.dat",
            "ped.dat"
    )
    var userID: ByteArray? = null
    var currentFileIndex = 0;
    lateinit var userAdapter: UserViewAdapter
    var pkgTotal = 0;
    var currentPkg = 0;
    var fileData: ByteArray? = null
    val model: LeftHead by viewModels()
    lateinit var userInfo: UserFile.UserInfo
    lateinit var leftHeadIcon: ImageView
    lateinit var leftName: TextView









    override fun onPause() {
        super.onPause()
        this.finish()
    }

    private fun addLiveDateObserver() {
        model.headIcon.observe(this, {
            leftHeadIcon.setImageResource(Constant.ICO_IMG[it - 1])
        })
        model.headName.observe(this, {
            leftName.text = it
        })
    }

    private fun initDrawer() {
        val toolbar: Toolbar = findViewById(R.id.toolbar)
        setSupportActionBar(toolbar)
        val drawerLayout: DrawerLayout = findViewById(R.id.drawer_layout)
        val navView: NavigationView = findViewById(R.id.nav_view)
        val navController = findNavController(R.id.nav_host_fragment)
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        appBarConfiguration = AppBarConfiguration(
                setOf(
                    R.id.nav_1, R.id.nav_2, R.id.nav_3
                ), drawerLayout
        )
        setupActionBarWithNavController(navController, appBarConfiguration)
        navView.setupWithNavController(navController)
        val headerLayout: View = navView.getHeaderView(0)
        leftHeadIcon = headerLayout.findViewById(R.id.imageView)
        leftName = headerLayout.findViewById(R.id.userName)
        leftHeadIcon.setOnClickListener {
            drawerLayout.closeDrawer(GravityCompat.START)
            drawerLayout.openDrawer(GravityCompat.END)
        }
    }

    private fun initActionBarColor() {
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        getWindow().setStatusBarColor(Color.BLACK);
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        initActionBarColor()
        setContentView(R.layout.activity_main)
        initDrawer()
        addLiveDateObserver()
        initVar()
        initView()
        initScan()
    }

    private val leScanCallback: ScanCallback = object : ScanCallback() {
        override fun onScanResult(
                callbackType: Int,
                result: ScanResult
        ) {
            super.onScanResult(callbackType, result)
            val device = result.device
            if (device == null) return;
            if (device.name == null) return;
            System.out.println(device.name)
            if (!device.name.contains("Checkme")) return;
            var z: Int = 0;
            var k: Int = 0;
            for (ble in bleList) run {
                if (ble.name.equals(device.name)) {
                    z = 1
                }
            }
            if (z == 0) {
                bleList.add(BleBean(device.name, device))
                bleViewAdapter.addDevice(device.name, device)
                Log.e("sdf", device.name)
            }
        }

        override fun onBatchScanResults(results: List<ScanResult>) {}
        override fun onScanFailed(errorCode: Int) {}
    }

    fun initScan() {
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

    private fun initView() {
        ble_table.layoutManager = GridLayoutManager(this, 2);
        bleViewAdapter = BleViewAdapter(this)
        ble_table.adapter = bleViewAdapter
        bleViewAdapter.setClickListener(this)

        val linearLayoutManager = LinearLayoutManager(this)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        userAdapter = UserViewAdapter(this, right_user)
        right_user.adapter = userAdapter
        right_user.layoutManager = linearLayoutManager
        userAdapter.setClickListener(this)


    }

    private fun initVar() {
        Constant.initVar(this)
        myBleManager = FDABleManager(this)
        myBleManager.setNotifyListener(this)
    }

    override fun onScanItemClick(bluetoothDevice: BluetoothDevice?) {
        bluetoothDevice?.let {
            myBleManager.connect(it)
                    .useAutoConnect(true)
                    .timeout(10000)
                    .retry(5, 100)
                    .done {
                        if (cmdState == 0) {
                            cmdState = 1
                            currentFileName="usr.dat"
                            val pkg = StartReadPkg(currentFileName)
                            sendCmd(pkg.buf)
                        }
                        Log.i("BLE", "连接成功了.>>.....>>>>")
                    }
                    .enqueue()
            leScanner.stopScan(leScanCallback)
            runOnUiThread {
                scan_title.visibility = GONE
                ble_table.visibility = GONE
                ble_panel.visibility = VISIBLE
                scan_layout.visibility = GONE
            }
        }
    }






    var sum: Int = 0;
    private var pool: ByteArray? = null
    override fun onNotify(device: BluetoothDevice?, data: Data?) {
        data?.value?.apply {
            pool = add(pool, this)
        }
        pool?.apply {
            pool = hasResponse(pool)
        }
    }

    private fun hasResponse(bytes: ByteArray?): ByteArray? {
        val bytesLeft: ByteArray? = bytes

        if (bytes == null || bytes.size < 8) {
            return bytes
        }
        loop@ for (i in 0 until bytes.size - 7) {
            if (bytes[i] != 0x55.toByte() || bytes[i + 1] != bytes[i + 2].inv()) {
                continue@loop
            }

            // need content length
            val len = toUInt(bytes.copyOfRange(i + 5, i + 7))
            if (i + 8 + len > bytes.size) {
                continue@loop
            }

            val temp: ByteArray = bytes.copyOfRange(i, i + 8 + len)
            if (temp.last() == calCRC8(temp)) {
                var s: String = "   "
                for (k in temp) {
                    val ga: Int = k.toUByte().toInt()
                    s += (ga.toString() + "   ");
//                    System.out.println(ga!!.toString())
                }
                sum += temp.size
                runOnUiThread {
                    notifySum.text = sum.toString()
                    notifyVal.text = s
                }
                val bleResponse = FDAResponse.CheckMeResponse(temp)
                if (cmdState == 1) {
                    fileData = null
                    pkgTotal = toUInt(bleResponse.content) / 1024
                    if (bleResponse.cmd == 1) {
                        val pkg = EndReadPkg()
                        sendCmd(pkg.buf)
                        cmdState = 3
                    } else if (bleResponse.cmd == 0) {
                        val pkg = ReadContentPkg(currentPkg)
                        sendCmd(pkg.buf)
                        currentPkg++
                        cmdState = 2;
                    }


                } else if (cmdState == 2) {
                    bleResponse.content.apply {
                        fileData = add(fileData, this)
                    }

                    if (currentPkg > pkgTotal) {
                        fileData?.apply {
                            if (currentFileIndex == 0) {
                                userInfo = UserFile.UserInfo(this)
                                for (user in userInfo.user) {
                                    userAdapter.addUser(user)
                                }
                            }
                            Thread {
                                sleep(300)
                                userAdapter.setUser(0)
                                runOnUiThread {
                                    onUserItemClick(userAdapter.mUserData[0], 0)
                                }

                            }.start()

                            File(getPathX(currentFileName)).writeBytes(this)
                        }
                        val pkg = EndReadPkg()
                        sendCmd(pkg.buf)
                        cmdState = 3
                    } else {
                        val pkg = ReadContentPkg(currentPkg)
                        sendCmd(pkg.buf)
                        currentPkg++
                    }

                } else if (cmdState == 3) {
                    fileData = null
                    currentPkg = 0;
                    cmdState = 0;
                    sum = 0
                }
                val tempBytes: ByteArray? = if (i + 8 + len == bytes.size) null else bytes.copyOfRange(
                        i + 8 + len,
                        bytes.size
                )

                return hasResponse(tempBytes)
            }
        }

        return bytesLeft
    }


    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment)
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
    }

    override fun onUserItemClick(userBean: UserBean?, position: Int) {
        userBean?.apply {
            model.headIcon.value = ico
            model.headName.value = name
            rName.text = name
            if (sex == 0) {
                rSex.text = "Male"
            } else {
                rSex.text = "Female"
            }
            val dateFormat = SimpleDateFormat("MMM. d, yyyy", ENGLISH)
            rBirthday.text = dateFormat.format(birthday)
            rWeight.text = weight.toString()
            rHeight.text = height.toString()
            rMedicalID.text = medicalId
            if (pacemakeflag == 0) {
                rPacemaker.text = "NO"
            } else {
                rPacemaker.text = "YES"
            }


        }

    }


    companion object {
        lateinit var myBleManager: FDABleManager
        fun sendCmd(bs: ByteArray) {
            myBleManager.sendCmd(bs)
        }

        var vibrator: Vibrator? = null
    }

}