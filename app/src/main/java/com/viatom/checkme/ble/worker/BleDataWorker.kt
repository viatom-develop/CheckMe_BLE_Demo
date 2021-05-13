package com.viatom.checkme.ble.worker

import android.bluetooth.BluetoothDevice
import android.content.Context
import android.util.Log
import com.viatom.checkme.ble.format.CheckMeResponse
import com.viatom.checkme.ble.format.DeviceInfo
import com.viatom.checkme.ble.manager.BleDataManager
import com.viatom.checkme.ble.pkg.EndReadPkg
import com.viatom.checkme.ble.pkg.GetDeviceInfoPkg
import com.viatom.checkme.ble.pkg.ReadContentPkg
import com.viatom.checkme.ble.pkg.StartReadPkg
import com.viatom.checkme.utils.CRCUtils
import com.viatom.checkme.utils.Constant
import com.viatom.checkme.utils.add
import com.viatom.checkme.utils.toUInt
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withTimeoutOrNull
import no.nordicsemi.android.ble.data.Data
import java.io.File
import kotlin.experimental.inv

class BleDataWorker {
    private var pool: ByteArray? = null
    private val deviceChannel = Channel<DeviceInfo>(Channel.CONFLATED)
    private val fileChannel = Channel<Int>(Channel.CONFLATED)
    private val connectChannel = Channel<String>(Channel.CONFLATED)
    private lateinit var myBleDataManager: BleDataManager
    private val dataScope = CoroutineScope(Dispatchers.IO)
    private val mutex = Mutex()

    private var cmdState = 0;
    var pkgTotal = 0;
    var currentPkg = 0;
    var fileData: ByteArray? = null
    var currentFileName = ""
    var result = 1;
    var currentFileSize = 0


    companion object {
        val fileProgressChannel = Channel<FileProgress>(Channel.CONFLATED)
    }

    data class FileProgress(
        var name: String = "",
        var progress: Int = 0,
        var success: Boolean = false
    )

    private val comeData = object : BleDataManager.OnNotifyListener {
        override fun onNotify(device: BluetoothDevice?, data: Data?) {
            data?.value?.apply {
                pool = add(pool, this)
            }
            pool?.apply {
                pool = handleDataPool(pool)
            }
        }

    }


    private fun handleDataPool(bytes: ByteArray?): ByteArray? {
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
            if (temp.last() == CRCUtils.calCRC8(temp)) {

                if (cmdState in 1..3) {
                    val bleResponse = CheckMeResponse(temp)
                    if (cmdState == 1) {
                        fileData = null
                        currentFileSize = toUInt(bleResponse.content)
                        pkgTotal = currentFileSize / 512
                        if (bleResponse.cmd == 1) {
                            result = 1
                            val pkg = EndReadPkg()
                            sendCmd(pkg.buf)
                            cmdState = 3
                        } else if (bleResponse.cmd == 0) {
                            val pkg =
                                ReadContentPkg(currentPkg)
                            sendCmd(pkg.buf)
                            currentPkg++
                            cmdState = 2;
                        }


                    } else if (cmdState == 2) {
                        bleResponse.content.apply {
                            fileData = add(fileData, this)
                            fileData?.let {
                                dataScope.launch {
                                    fileProgressChannel.send(
                                        FileProgress(
                                            currentFileName,
                                            it.size * 100 / currentFileSize,
                                            true
                                        )
                                    )
                                }
                            }
                        }

                        if (currentPkg > pkgTotal) {
                            fileData?.apply {
                                result = 0
                                Log.i("file", "receive  $currentFileName")
                                File(Constant.getPathX(currentFileName)).writeBytes(this)
                            }
                            val pkg = EndReadPkg()
                            sendCmd(pkg.buf)
                            cmdState = 3
                        } else {
                            val pkg =
                                ReadContentPkg(currentPkg)
                            sendCmd(pkg.buf)
                            currentPkg++
                        }

                    } else if (cmdState == 3) {
                        fileData = null
                        currentPkg = 0
                        cmdState = 0
                        dataScope.launch {
                            fileProgressChannel.send(
                                FileProgress(
                                    currentFileName,
                                    100,
                                    result == 0
                                )
                            )
                            fileChannel.send(result)
                        }
                    }
                } else if (cmdState == 4) {
                    val deviceInfo = DeviceInfo(temp)
                    dataScope.launch {
                        deviceChannel.send(deviceInfo)
                    }
                }


                val tempBytes: ByteArray? =
                    if (i + 8 + len == bytes.size) null else bytes.copyOfRange(
                        i + 8 + len,
                        bytes.size
                    )

                return handleDataPool(tempBytes)
            }
        }

        return bytesLeft
    }

    private fun sendCmd(bs: ByteArray) {
        myBleDataManager.sendCmd(bs)
    }


    fun initWorker(context: Context, bluetoothDevice: BluetoothDevice?) {
        myBleDataManager = BleDataManager(context)
        myBleDataManager.setNotifyListener(comeData)
        bluetoothDevice?.let {
            myBleDataManager.connect(it)
                .useAutoConnect(true)
                .timeout(10000)
                .retry(15, 100)
                .done {
                    Log.i("BLE", "连接成功了.>>.....>>>>")
                    dataScope.launch {
                        connectChannel.send("yes")
                    }

                }
                .enqueue()
        }
    }

    suspend fun waitConnect() {
        connectChannel.receive()
    }

    suspend fun getFile(name: String): Int {
        mutex.withLock {
            this.currentFileName = name
            cmdState = 1
            val pkg = StartReadPkg(name)
            sendCmd(pkg.buf)
            return fileChannel.receive()
        }
    }

    suspend fun getDeviceInfo(): DeviceInfo {
        mutex.withLock {
            cmdState = 4
            val pkg = GetDeviceInfoPkg()
            sendCmd(pkg.buf)
            return deviceChannel.receive()
        }
    }

}