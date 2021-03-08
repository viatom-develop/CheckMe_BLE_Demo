package com.viatom.checkme.activity

import android.bluetooth.BluetoothDevice
import android.os.Bundle
import android.view.View
import android.view.View.GONE
import android.view.View.VISIBLE
import android.widget.ImageView
import android.widget.TextView
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.fragment.app.Fragment
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.navigation.NavigationView
import com.viatom.checkme.leftnavi.UiChannel
import com.viatom.checkme.R
import com.viatom.checkme.adapter.BleViewAdapter
import com.viatom.checkme.adapter.UserViewAdapter
import com.viatom.checkme.bean.BleBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.ble.format.UserFile
import com.viatom.checkme.ble.manager.BleScanManager
import com.viatom.checkme.ble.worker.BleDataWorker
import com.viatom.checkme.leftnavi.dailyCheck.DailyCheckFragment
import com.viatom.checkme.leftnavi.ecgRecorder.EcgRecorderFragment
import com.viatom.checkme.leftnavi.pedometer.PedometerFragment
import com.viatom.checkme.leftnavi.oximeter.OximiterFragment
import com.viatom.checkme.leftnavi.thermometer.TmpFragment
import com.viatom.checkme.utils.Constant
import com.viatom.checkme.viewmodel.LeftHead
import kotlinx.android.synthetic.main.right_drawer.*
import kotlinx.android.synthetic.main.scan_view.*
import kotlinx.coroutines.*
import kotlinx.coroutines.channels.Channel
import java.io.File
import java.lang.Thread.sleep
import java.text.SimpleDateFormat
import java.util.Locale.ENGLISH


class MainActivity : AppCompatActivity(), BleViewAdapter.ItemClickListener,
    UserViewAdapter.userClickListener,
    BleScanManager.Scan {
    companion object {
        var isOffline=false
        var loading = true
        var currentId = ""
        val bleWorker = BleDataWorker()
        val scan = BleScanManager()
        val dataScope = CoroutineScope(Dispatchers.IO)
        val uiScope = CoroutineScope(Dispatchers.Main)
    }

    lateinit var mMainNavFragment: Fragment
    private lateinit var appBarConfiguration: AppBarConfiguration
    private val bleList: MutableList<BleBean> = ArrayList()
    lateinit var bleViewAdapter: BleViewAdapter

    private val userChannel = Channel<Int>(Channel.CONFLATED)

    //-------------8 files
    var downloadNumber = 8
    var currentNumber = 0
    private var userfileName = arrayOf(
        "dlc.dat",
        "spc.dat",
        "hrv.dat",
        "ecg.dat",
        "oxi.dat",
        "tmp.dat",
        "slm.dat",
        "ped.dat"
    )
    var currentUser = 0

    lateinit var userAdapter: UserViewAdapter
    private val model: LeftHead by viewModels()

    @ExperimentalUnsignedTypes
    lateinit var userInfo: UserFile.UserInfo
    lateinit var leftHeadIcon: ImageView
    lateinit var leftName: TextView

    @ExperimentalUnsignedTypes
    private suspend fun readUser() {
        userChannel.receive()
        val userTemp = File(Constant.getPathX("usr.dat")).readBytes()
        userTemp.apply {
            userInfo = UserFile.UserInfo(this)
            val total=userInfo.user.size*userfileName.size+1
            var tIndex=1
            UiChannel.progressChannel.send(tIndex*100/total)
            for (user in userInfo.user) {
                for (f in userfileName) {
                    bleWorker.getFile(user.id + f)
                    tIndex++
                    UiChannel.progressChannel.send(tIndex*100/total)
                }
                userAdapter.addUser(user)
            }
            delay(300)
            UiChannel.progressChannel.close()
            userAdapter.setUserColor(0)
            onUserItemClick(userAdapter.mUserData[0], 0)
            loading = false
        }
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
                R.id.nav_1,
                R.id.nav_2,
                R.id.nav_3,
                R.id.nav_4,
                R.id.nav_5,
                R.id.nav_51,
                R.id.nav_6,
                R.id.nav_7
            ), drawerLayout
        )
        mMainNavFragment = supportFragmentManager.findFragmentById(R.id.nav_host_fragment)!!
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
    }


    fun initScan() {
        scan.initScan(this)
        scan.setCallBack(this)
    }

    override fun onBackPressed() {
        moveTaskToBack(false)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        initDrawer()
        addLiveDateObserver()
        initVar()
        initView()
        initScan()
    }


    override fun onSupportNavigateUp(): Boolean {
        val navController = findNavController(R.id.nav_host_fragment)
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
    }

    @ExperimentalUnsignedTypes
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

            val fragmentA = mMainNavFragment.childFragmentManager.primaryNavigationFragment
            if (fragmentA is DailyCheckFragment) {
                fragmentA.switch(id)
            } else if (fragmentA is EcgRecorderFragment) {
                fragmentA.switch(id)
            } else if (fragmentA is PedometerFragment) {
                fragmentA.switch(id)
            } else if (fragmentA is OximiterFragment) {
                fragmentA.switch(id)
            } else if (fragmentA is TmpFragment) {
                fragmentA.switch(id)
            }
            currentId = id
        }

    }


    @ExperimentalUnsignedTypes
    override fun onScanItemClick(bluetoothDevice: BluetoothDevice?) {
        scan.stop()
        bleWorker.initWorker(this, bluetoothDevice)
        runOnUiThread {
            scan_title.visibility = GONE
            ble_table.visibility = GONE
            ble_panel.visibility = VISIBLE
            scan_layout.visibility = GONE
        }
        dataScope.launch {
            val a = withTimeoutOrNull(10000) {
                bleWorker.waitConnect()
            }
            a?.let {
                val b = withTimeoutOrNull(10000) {
                    bleWorker.getFile("usr.dat")
                }
                b?.let {
                    userChannel.send(1)
                }


            }
        }
        uiScope.launch {
            readUser()
        }


    }

    override fun scanReturn(name: String, bluetoothDevice: BluetoothDevice) {
        if (!(name.contains("Checkme"))) return;
        var z: Int = 0;
        for (ble in bleList) run {
            if (ble.name.equals(bluetoothDevice.name)) {
                z = 1
            }
        }
        if (z == 0) {
            bleList.add(BleBean(name, bluetoothDevice))
            bleViewAdapter.addDevice(name, bluetoothDevice)
        }
    }

    @ExperimentalUnsignedTypes
    fun offline(view: View) {
        isOffline=true
        scan.stop()
        runOnUiThread {
            scan_title.visibility = GONE
            ble_table.visibility = GONE
            ble_panel.visibility = VISIBLE
            scan_layout.visibility = GONE
        }
        MainActivity.loading=false
        val file=File(Constant.getPathX("usr.dat"))
        if(file.exists()){
            val userTemp = File(Constant.getPathX("usr.dat")).readBytes()
            userTemp.apply {
                userInfo = UserFile.UserInfo(this)
                for (user in userInfo.user) {
                    userAdapter.addUser(user)
                }
                sleep(300)
                userAdapter.setUserColor(0)
                onUserItemClick(userAdapter.mUserData[0], 0)
                loading = false
            }
        }
        UiChannel.progressChannel.close()

    }

}