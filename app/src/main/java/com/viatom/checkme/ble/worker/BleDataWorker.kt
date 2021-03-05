package com.viatom.checkme.ble.worker

import android.bluetooth.BluetoothDevice
import android.content.Context
import android.util.Log
import com.vaca.x1.utils.add
import com.vaca.x1.utils.toUInt
import com.viatom.checkme.ble.manager.BleDataManager
import com.viatom.checkme.ble.pkg.EndReadPkg
import com.viatom.checkme.ble.pkg.FDAResponse
import com.viatom.checkme.ble.pkg.ReadContentPkg
import com.viatom.checkme.ble.pkg.StartReadPkg
import com.viatom.checkme.utils.CRCUtils
import com.viatom.checkme.utils.Constant
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.io.File
import kotlin.experimental.inv

class BleDataWorker {
    private var pool: ByteArray? = null
    private val fileChannel = Channel<Int>(Channel.CONFLATED)
    private val connectChannel = Channel<String>(Channel.CONFLATED)
    private lateinit var myBleDataManager: BleDataManager
    val dataScope = CoroutineScope(Dispatchers.IO)
    private var cmdState = 0;
    var pkgTotal = 0;
    var currentPkg = 0;
    var fileData: ByteArray? = null
    var currentFileName=""
    var result=1;

    private val comeData = BleDataManager.onNotifyListener { device, data ->
        data?.value?.apply {
            pool = add(pool, this)
        }
        pool?.apply {
            pool = hasResponse(pool)
        }
    }

    @ExperimentalUnsignedTypes
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
            if (temp.last() == CRCUtils.calCRC8(temp)) {
                val bleResponse = FDAResponse.CheckMeResponse(temp)
                if (cmdState == 1) {
                    fileData = null
                    pkgTotal = toUInt(bleResponse.content) / 1024
                    if (bleResponse.cmd == 1) {
                        result=1
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
                    }

                    if (currentPkg > pkgTotal) {
                        fileData?.apply {
                            result=0
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
                        fileChannel.send(result)
                    }
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
                .retry(5, 100)
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

    fun getFile(name: String):Int {
        this.currentFileName=name
        var result=1
        runBlocking {
            cmdState=1
            val pkg = StartReadPkg(name)
            sendCmd(pkg.buf)
            result= fileChannel.receive()
        }
        return result
    }

}