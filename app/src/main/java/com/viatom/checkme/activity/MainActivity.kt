package com.viatom.checkme.activity

import android.bluetooth.BluetoothDevice
import android.os.Bundle
import android.view.View
import android.view.View.GONE
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
import com.viatom.checkme.R
import com.viatom.checkme.adapter.BleViewAdapter
import com.viatom.checkme.adapter.UserViewAdapter
import com.viatom.checkme.bean.BleBean
import com.viatom.checkme.bean.UserBean
import com.viatom.checkme.ble.format.UserInfo
import com.viatom.checkme.ble.manager.BleScanManager
import com.viatom.checkme.ble.worker.BleDataWorker
import com.viatom.checkme.databinding.ActivityMainBinding
import com.viatom.checkme.databinding.RightDrawerBinding
import com.viatom.checkme.databinding.ScanViewBinding
import com.viatom.checkme.leftnavi.UiChannel
import com.viatom.checkme.leftnavi.bp.BpFragment
import com.viatom.checkme.leftnavi.dailyCheck.DailyCheckFragment
import com.viatom.checkme.leftnavi.ecgRecorder.EcgRecorderFragment
import com.viatom.checkme.leftnavi.oximeter.OximiterFragment
import com.viatom.checkme.leftnavi.pedometer.PedometerFragment
import com.viatom.checkme.leftnavi.sleepMonitor.SleepFragment
import com.viatom.checkme.leftnavi.thermometer.TmpFragment
import com.viatom.checkme.utils.Constant
import com.viatom.checkme.viewmodel.LeftHead
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
        var isOffline = false
        var loading = true
        var currentId = ""
        val bleWorker = BleDataWorker()
        val scan = BleScanManager()
        val dataScope = CoroutineScope(Dispatchers.IO)
        val uiScope = CoroutineScope(Dispatchers.Main)
    }

    private lateinit var binding: ActivityMainBinding
    private lateinit var bindingScan: ScanViewBinding
    private lateinit var bindingRight: RightDrawerBinding
    lateinit var mMainNavFragment: Fragment
    private lateinit var appBarConfiguration: AppBarConfiguration
    private val bleList: MutableList<BleBean> = ArrayList()
    lateinit var bleViewAdapter: BleViewAdapter

    private val userChannel = Channel<Int>(Channel.CONFLATED)

    //-------------8 files
    var downloadNumber = 8
    var currentNumber = 0
    private var userFileName = arrayOf(
        "dlc.dat",
        "spc.dat",
        "hrv.dat",
        "ecg.dat",
        "oxi.dat",
        "tmp.dat",
        "bpi.dat",
        "slm.dat",
        "ped.dat",
        "nibp.dat"
    )
    var currentUser = 0

    lateinit var userAdapter: UserViewAdapter
    private val model: LeftHead by viewModels()


    lateinit var userInfo: UserInfo
    private lateinit var leftHeadIcon: ImageView
    lateinit var leftName: TextView
    lateinit var iconSync: ImageView
    lateinit var nameTop: TextView


    private suspend fun readUser() {
        userChannel.receive()
        val userTemp = File(Constant.getPathX("usr.dat")).readBytes()
        userTemp.apply {
            userInfo = UserInfo(this)
            val total = userInfo.user.size * userFileName.size + 1
            var tIndex = 1
            UiChannel.progressChannel.send(tIndex * 100 / total)
            for (user in userInfo.user) {
                for (f in userFileName) {
                    bleWorker.getFile(user.id + f)
                    tIndex++
                    UiChannel.progressChannel.send(tIndex * 100 / total)
                }
                userAdapter.addUser(user)
            }

//            val xx=File(Constant.getPathX("2nibp.dat")).readBytes()
//            for(k in xx){
//                println("fuck"+k.toUByte().toInt().toString())
//            }
            delay(300)
            UiChannel.progressChannel.close()
            userAdapter.setUserColor(0)
            onUserItemClick(userAdapter.mUserData[0], 0)
            loading = false
        }
    }


    private fun addLiveDateObserver() {
        model.headIcon.observe(this, {
            iconSync.setImageResource(Constant.ICO_IMG[it - 1])
            leftHeadIcon.setImageResource(Constant.ICO_IMG[it - 1])
        })
        model.headName.observe(this, {
            leftName.text = it
            nameTop.text = it
        })


    }

    private fun initDrawer() {
        val toolbar: Toolbar = findViewById(R.id.toolbar)
        iconSync = findViewById(R.id.icon_sync)
        nameTop = findViewById(R.id.userTop)
        setSupportActionBar(toolbar)
        val drawerLayout: DrawerLayout = findViewById(R.id.drawer_layout)
        val navView: NavigationView = findViewById(R.id.nav_view)
        val navController = findNavController(R.id.nav_host_fragment)
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        appBarConfiguration = AppBarConfiguration(
            setOf(
                R.id.nav_1,
                R.id.bpFragment,
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
        iconSync.setOnClickListener {
            drawerLayout.closeDrawer(GravityCompat.START)
            drawerLayout.openDrawer(GravityCompat.END)
        }
    }


    private fun initView() {
        bindingScan.bleTable.layoutManager = GridLayoutManager(this, 2);
        bleViewAdapter = BleViewAdapter(this)
        bindingScan.bleTable.adapter = bleViewAdapter
        bleViewAdapter.setClickListener(this)

        val linearLayoutManager = LinearLayoutManager(this)
        linearLayoutManager.orientation = RecyclerView.VERTICAL
        userAdapter = UserViewAdapter(this, bindingRight.rightUser)
        bindingRight.rightUser.adapter = userAdapter
        bindingRight.rightUser.layoutManager = linearLayoutManager
        userAdapter.setClickListener(this)
    }

    private fun initVar() {
        Constant.initVar(this)
    }


    private fun initScan() {
        scan.initScan(this)
        scan.setCallBack(this)
    }

    override fun onBackPressed() {
        moveTaskToBack(false)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        binding.scanView.also { bindingScan = it }
        binding.rightDrawer.also { bindingRight = it }
        val view = binding.root
        setContentView(view)
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


    override fun onUserItemClick(userBean: UserBean?, position: Int) {
        userBean?.apply {
            model.headIcon.value = ico
            model.headName.value = name
            bindingRight.apply {
                rName.text = name
                if (sex == 0) {
                    rSex.text = getString(R.string.male)
                } else {
                    rSex.text = getString(R.string.female)
                }
                val dateFormat = SimpleDateFormat("MMM. d, yyyy", ENGLISH)
                rBirthday.text = dateFormat.format(birthday)
                rWeight.text = weight.toString()
                rHeight.text = height.toString()
                rMedicalID.text = medicalId
                if (pacemakeflag == 0) {
                    rPacemaker.text = getString(R.string.no)
                } else {
                    rPacemaker.text = getString(R.string.yes)
                }
            }


            val fragmentCurrent = mMainNavFragment.childFragmentManager.primaryNavigationFragment
            if (fragmentCurrent is DailyCheckFragment) {
                fragmentCurrent.switch(id)
            } else if (fragmentCurrent is EcgRecorderFragment) {
                fragmentCurrent.switch(id)
            } else if (fragmentCurrent is PedometerFragment) {
                fragmentCurrent.switch(id)
            } else if (fragmentCurrent is OximiterFragment) {
                fragmentCurrent.switch(id)
            } else if (fragmentCurrent is TmpFragment) {
                fragmentCurrent.switch(id)
            } else if (fragmentCurrent is SleepFragment) {
                fragmentCurrent.switch(id)
            }else if(fragmentCurrent is BpFragment){
                fragmentCurrent.switch(id)
            }

            currentId = id
        }

    }


    override fun onScanItemClick(bluetoothDevice: BluetoothDevice?) {
        scan.stop()
        bleWorker.initWorker(this, bluetoothDevice)
        runOnUiThread {
            bindingScan.apply {
                scanTitle.visibility = GONE
                bleTable.visibility = GONE
                scanLayout.visibility = GONE
            }

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
            if (ble.name == bluetoothDevice.name) {
                z = 1
            }
        }
        if (z == 0) {
            bleList.add(BleBean(name, bluetoothDevice))
            bleViewAdapter.addDevice(name, bluetoothDevice)
        }
    }


    fun View.offline() {
        isOffline = true
        scan.stop()
        runOnUiThread {
            bindingScan.apply {
                scanTitle.visibility = GONE
                bleTable.visibility = GONE
                scanLayout.visibility = GONE
            }

        }
        MainActivity.loading = false
        val file = File(Constant.getPathX("usr.dat"))
        if (file.exists()) {
            val userTemp = File(Constant.getPathX("usr.dat")).readBytes()
            userTemp.apply {
                userInfo = UserInfo(this)
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