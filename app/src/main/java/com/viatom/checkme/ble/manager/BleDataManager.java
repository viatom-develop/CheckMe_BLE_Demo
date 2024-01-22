package com.viatom.checkme.ble.manager;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.util.Log;

import java.util.UUID;

import no.nordicsemi.android.ble.BleManager;
import no.nordicsemi.android.ble.data.Data;

public class BleDataManager extends BleManager {
    private BluetoothGattCharacteristic write_char;
    private BluetoothGattCharacteristic notify_char;
    private OnNotifyListener listener;

    public BleDataManager(Context context) {
        super(context);
    }

    public void setNotifyListener(OnNotifyListener listener) {
        this.listener = listener;
    }

    @Override
    protected BleManagerGattCallback getGattCallback() {
        return new MyManagerGattCallback();
    }

    public void sendCmd(byte[] bytes) {
        writeCharacteristic(write_char, bytes)
                .split()
                .done(device -> {
                })
                .enqueue();
    }


    public interface OnNotifyListener {
        void onNotify(BluetoothDevice device, Data data);
    }

    private class MyManagerGattCallback extends BleManagerGattCallback {
        @Override
        public boolean isRequiredServiceSupported(BluetoothGatt gatt) {
            final BluetoothGattService service = gatt.getService(service_uuid);


            if (service != null) {
                write_char = service.getCharacteristic(write_uuid);
                notify_char = service.getCharacteristic(notify_uuid);
            }

            boolean notify = false;
            if (notify_char != null) {
                final int properties = notify_char.getProperties();
                notify = (properties & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0;
            }
            boolean writeRequest = false;
            if (write_char != null) {
                final int properties = write_char.getProperties();
                int writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT;
                if ((properties & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) != 0) {
                    writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE;
                }
                write_char.setWriteType(writeType);
                writeRequest = (properties & BluetoothGattCharacteristic.PROPERTY_WRITE) != 0 || (properties & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) != 0;
                ///     LepuBleLog.d(TAG, "writeChar writeRequest ==  " + writeRequest);

            }
            // Return true if all required services have been found
            return write_char != null && notify_char != null
                    && notify && writeRequest;
        }

        @Override
        public boolean isOptionalServiceSupported(BluetoothGatt gatt) {
            return super.isOptionalServiceSupported(gatt);
        }

        @Override
        protected void initialize() {
            beginAtomicRequestQueue()
                    .add(requestMtu(23) // Remember, GATT needs 3 bytes extra. This will allow packet size of 244 bytes.
                            .with((device, mtu) -> Log.d("TAG", "MTU set to " + mtu))
                            .fail((device, status) -> log(Log.WARN, "Requested MTU not supported: " + status)))
                    .add(enableNotifications(notify_char))
                    .done(device -> Log.d("TAG", "Target initialized"))
                    .enqueue();
            setNotificationCallback(notify_char)
                    .with((device, data) -> {
                        listener.onNotify(device, data);
                    });
        }

        @Override
        public void onDeviceDisconnected() {
            write_char = null;
            notify_char = null;
        }
    }

    public static final UUID service_uuid= UUID.fromString("14839ac4-7d7e-415c-9a42-167340cf2339");
    public static final UUID write_uuid =UUID.fromString("8B00ACE7-EB0B-49B0-BBE9-9AEE0A26E1A3");
    public static final UUID notify_uuid=  UUID.fromString("0734594A-A8E7-4B1A-A6B1-CD5243059A57");
}