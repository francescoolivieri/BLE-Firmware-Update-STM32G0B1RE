import binascii
from enum import Enum

import bincopy
from PySide6.QtCore import (Qt, QEvent, QObject, Signal, Slot)
from PySide6.QtWidgets import (QApplication, QLabel, QMainWindow, QPushButton, QVBoxLayout, QWidget, QLineEdit,
                               QProgressBar, QTextEdit, QFileDialog, QMessageBox, QHBoxLayout, QRadioButton,
                               QButtonGroup)
from PySide6.QtGui import QFont, QTextCursor, QIcon, QImage, QPixmap

import asyncio
import signal
import sys
import time

from Crypto.Cipher import AES

from bleak import BleakClient, BleakScanner

TNS_UUID = "984f3988-b8ae-49f4-9d89-fc6b2b987b88"
RCV_UUID = "984f3988-b8ae-4af4-9d89-fc6b2b987b88"

key = bytearray([0x46, 0x3b, 0x41, 0x29, 0x11, 0x76, 0x7d, 0x57, 0xa0, 0xb3, 0x39, 0x69, 0xe6, 0x74, 0xff, 0xe7, 0x84,
                 0x5d, 0x31, 0x3b, 0x88, 0xc6, 0xfe, 0x31, 0x2f, 0x3d, 0x72, 0x4b, 0xe6, 0x8e, 0x1f, 0xca])

# Define the nonce or IV
iv = bytearray([0x61, 0x1c, 0xe6, 0xf9, 0xa6, 0x88, 0x07, 0x50, 0xde, 0x7d, 0xa6, 0xcb])

ack_rcv = False
is_ack = True
msg_tampered = False

rand_IV = [0, 0, 0]
tag_len = 4
security = False


def restore_crypto_utils():
    global key
    global iv

    key = bytearray([0x46, 0x3b, 0x41, 0x29, 0x11, 0x76, 0x7d, 0x57, 0xa0, 0xb3, 0x39, 0x69, 0xe6, 0x74, 0xff, 0xe7, 0x84,
                 0x5d, 0x31, 0x3b, 0x88, 0xc6, 0xfe, 0x31, 0x2f, 0x3d, 0x72, 0x4b, 0xe6, 0x8e, 0x1f, 0xca])
    iv = bytearray([0x61, 0x1c, 0xe6, 0xf9, 0xa6, 0x88, 0x07, 0x50, 0xde, 0x7d, 0xa6, 0xcb])


# Funzione per cifrare un messaggio
def encrypt_message(plaintext, add_data, nonce):
    cipher = AES.new(key, AES.MODE_GCM, nonce=nonce, mac_len=tag_len)
    cipher.update(add_data)
    ciphertext, tag = cipher.encrypt_and_digest(plaintext)
    return ciphertext, tag


# Funzione per decifrare un messaggio
def decrypt_message(ciphertext, add_data, tag, key, nonce):
    print(f"ciphertext:{ciphertext.hex()}, add_data:{add_data.hex()}, tag:{tag.hex()}, iv:{nonce.hex()}")

    cipher = AES.new(key, AES.MODE_GCM, nonce=nonce, mac_len=tag_len)
    cipher.update(add_data)
    try:
        decrypted_message = cipher.decrypt(ciphertext)
        cipher.verify(tag)

        return decrypted_message
    except ValueError:
        print("Verification FAILED!")
        return False


async def read_response(sender, data):
    global ack_rcv
    global is_ack
    global security
    global rand_IV
    global iv
    global msg_tampered

    str = binascii.hexlify(bytearray(data))
    print(str)
    if not security:

        if data[0] != 0x00 :
            is_ack = False
            print("NAK RECEIVED!")

        else:
            is_ack = True
    else:
        dec_payload = decrypt_message(data[1:6], data[0:1], data[6:], key, iv)

        if dec_payload:
            print("Correct ACK received")

            msg_tampered = False
            if (data[0] >> 4) == 0x0:

                rand_IV = dec_payload[2:]

                is_ack = True
            else:

                is_ack = False

        else:
            msg_tampered = True

            print("Security broke")

    ack_rcv = True


class ConnectionStatus(Enum):
    IDLE = 0
    INIT_PHASE = 1
    INIT_CRYPTO_PHASE = 2
    SENDING_DATA = 3
    SENDING_CRYPTO_DATA = 4
    CLOSING_PHASE = 5


class AppSelected(Enum):
    APP_FULL_SPACE = 0
    APP_1 = 1
    APP_2 = 2


class MainWindow(QMainWindow):
    start_signal = Signal()
    done_signal = Signal()

    end_fw_update = False

    def __init__(self):
        super().__init__()

        # Set window properties
        self.setWindowTitle("Firmware Uploader")
        self.setFixedSize(400, 280)

        font_main = QFont("Arial", 10)
        self.setWindowIcon(QIcon("icona_app_fw_upload.png"))
        font_main.setBold(True)

        # Create widgets
        self.fw_file_label = QLabel("Firmware file:")
        self.fw_file_label.setFont(font_main)
        self.fw_file_textbox = QLineEdit()
        self.fw_file_textbox.setReadOnly(True)
        self.fw_file_button = QPushButton("Select file")
        self.fw_file_button.clicked.connect(self.select_fw_file)
        self.fw_file_button.setFont(QFont("Arial", 8))

        self.device_address_label = QLabel("Device address:")
        self.device_address_label.setFont(font_main)
        self.device_address_textbox = QLineEdit("06:05:04:03:02:01")
        self.device_address_textbox.setFont(QFont("Arial", 12))
        self.device_address_textbox.setAlignment(Qt.AlignCenter)

        self.conn_type_label = QLabel("Connection Type: ")
        self.conn_type_label.setFont(font_main)

        self.secure_conn_checkbox = QRadioButton("Secure", self)
        self.secure_conn_checkbox.click()
        self.raw_conn_checkbox = QRadioButton("Raw", self)

        self.app_sel = QButtonGroup(self)
        self.app_full_space_radiobutton = QRadioButton("App (256 KB)")
        self.app_1_radiobutton = QRadioButton("App_1 (128 KB)")
        self.app_2_radiobutton = QRadioButton("App_2 (128 KB)")

        self.app_sel.addButton(self.app_full_space_radiobutton, 0)
        self.app_sel.addButton(self.app_1_radiobutton, 1)
        self.app_sel.addButton(self.app_2_radiobutton, 2)

        self.progress_bar = QProgressBar()
        self.progress_bar.setVisible(True)
        self.progress_bar.setMinimum(1)
        self.progress_bar.setMaximum(100)
        self.progress_bar.setTextVisible(False)

        self.log_textbox = QTextEdit()
        self.log_textbox.setReadOnly(True)
        self.log_textbox.setFixedSize(383, 80)

        self.upload_button = QPushButton("Upload firmware")
        self.upload_button.clicked.connect(self.upload_firmware)
        self.upload_button.setFont(QFont("Arial", 8))

        # Set widget properties
        self.fw_file_label.setAlignment(Qt.AlignVCenter | Qt.AlignVCenter)
        self.device_address_label.setAlignment(Qt.AlignVCenter | Qt.AlignVCenter)

        # Create layout
        central_widget = QWidget()
        layout = central_widget.layout()
        if layout is None:
            layout = QVBoxLayout()

        layout_fw_selection = QVBoxLayout()
        layout_fw_selection_top = QHBoxLayout()
        layout_fw_selection_top.addSpacing(20)
        layout_fw_selection_top.addWidget(self.fw_file_label)
        layout_fw_selection_top.addWidget(self.fw_file_button)
        layout.addLayout(layout_fw_selection_top)
        layout_fw_selection.addWidget(self.fw_file_textbox)

        layout.addLayout(layout_fw_selection)

        layout.addStretch(5)
        layout3 = QHBoxLayout()
        layout3.addSpacing(20)
        layout3.addWidget(self.device_address_label)
        layout3.addWidget(self.device_address_textbox)
        layout.addLayout(layout3)

        layout.addStretch(10)
        layout4 = QHBoxLayout()
        layout4.addWidget(self.conn_type_label)
        layout4.addWidget(self.secure_conn_checkbox)
        layout4.addWidget(self.raw_conn_checkbox)
        layout.addLayout(layout4)

        layout.addStretch(10)
        layout5 = QHBoxLayout()
        layout5.addWidget(self.app_full_space_radiobutton)
        layout5.addWidget(self.app_1_radiobutton)
        layout5.addWidget(self.app_2_radiobutton)
        layout.addLayout(layout5)

        layout.addStretch(10)
        layout.addWidget(self.upload_button)
        layout.addWidget(self.progress_bar)

        layout.addStretch(10)
        layout.addWidget(self.log_textbox)

        # Set central widget
        central_widget.setLayout(layout)
        self.setCentralWidget(central_widget)

    @Slot()
    def upload_firmware(self):
        self.fw_file = self.fw_file_textbox.text()
        self.device_address = self.device_address_textbox.text()

        if not self.fw_file:
            QMessageBox.warning(self, "Error", "Please select a firmware file")
            return

        if not self.device_address:
            QMessageBox.warning(self, "Error", "Please enter a device address")
            return

        self.progress_bar.setVisible(True)
        self.upload_button.setEnabled(False)

        self.start_signal.emit()

    def select_fw_file(self):
        fw_file, _ = QFileDialog.getOpenFileName(self, "Select firmware file (.srec)", "", "Firmware Files (*.srec)")
        if fw_file:
            self.fw_file_textbox.setText(fw_file)

    def update_progress(self, progress):
        self.progress_bar.setValue(progress)

    def update_log(self, message):
        self.log_textbox.insertPlainText(message)
        self.log_textbox.moveCursor(QTextCursor.End)

    def upload_finished(self):
        self.progress_bar.setVisible(False)
        self.upload_button.setEnabled(True)
        if self.progress_bar.value() != 100:
            QMessageBox.information(self, "Failed", "Firmware upload stopped")
        else:
            QMessageBox.information(self, "Success", "Firmware upload complete")

    async def send_new_fw(self):
        nack_loop = 0
        connection_status = ConnectionStatus.IDLE

        async with BleakClient(self.device_address_textbox.text()) as client:
            self.update_log("Connected\n")
            connection_status = ConnectionStatus.IDLE

            global ack_rcv  # notifies when an ack is received
            global is_ack
            global security
            global msg_tampered

            # prepare new firmware
            f = bincopy.BinFile()
            f.add_srec_file(self.fw_file_textbox.text())
            data = f.as_binary()
            data = bytearray(data)

            if self.secure_conn_checkbox.isChecked():
                connection_status = ConnectionStatus.INIT_CRYPTO_PHASE
                self.update_log("Connection crypted selected.\n")

                # prepare chunks of 15 bytes
                chunk_size = 15

                security = True
            else:
                connection_status = ConnectionStatus.INIT_PHASE
                self.update_log("Connection raw selected.\n")

                # prepare chunks of 16 bytes
                chunk_size = 16

                security = False

            chunks = [data[i:i + chunk_size] for i in range(0, len(data), chunk_size)]
            num_chunks = len(chunks)

            selected_app_update = self.app_sel.checkedId()

            if selected_app_update == AppSelected.APP_FULL_SPACE:
                if (num_chunks * chunk_size) / 256000 > 1:
                    QMessageBox.information(self, "FW size",
                                            "Firmware upload cannot start, size of the new code is too big")
                    connection_status = ConnectionStatus.CLOSING_PHASE

            elif selected_app_update == AppSelected.APP_1:
                if (num_chunks * chunk_size) / 128000 > 1:
                    QMessageBox.information(self, "FW size",
                                            "Firmware upload cannot start, size of the new code is too big")
                    connection_status = ConnectionStatus.CLOSING_PHASE

            elif selected_app_update == AppSelected.APP_2:
                if (num_chunks * chunk_size) / 128000 > 1:
                    QMessageBox.information(self, "FW size",
                                            "Firmware upload cannot start, size of the new code is too big")
                    connection_status = ConnectionStatus.CLOSING_PHASE

            await client.start_notify(RCV_UUID, read_response)  # client ready to receive data (ack/nak)
            # !!! SHOULD CHECK IF CONNECTED !!!

            start_t = time.time()

            # start logic of the program
            while connection_status != ConnectionStatus.CLOSING_PHASE:
                if connection_status == ConnectionStatus.INIT_PHASE:  # START FLASH MODE PACKET
                    self.update_log("Settings completed successfully\n")
                    # send start flash mode message
                    msg = bytearray()
                    msg += bytearray.fromhex("bb")
                    msg += selected_app_update.to_bytes(1, byteorder='big')
                    print(f"WEE  {selected_app_update}  e  {msg}")
                    msg += num_chunks.to_bytes(2, byteorder='big')
                    checksum = (msg[-2] + msg[-1]) & 0xFF
                    msg += checksum.to_bytes(1, byteorder='big')

                    # send pck
                    await client.write_gatt_char(TNS_UUID, msg, response=False)

                    # wait for ACK
                    while not ack_rcv:
                        await asyncio.sleep(0.005)
                        pass

                    ack_rcv = False

                    if not is_ack:
                        connection_status = ConnectionStatus.CLOSING_PHASE
                        QMessageBox.information(self, "FW Upload Request",
                                                "Firmware upload cannot start, the request has been denied by the target device")
                    else:
                        await asyncio.sleep(0.005)
                        connection_status = ConnectionStatus.SENDING_DATA

                elif connection_status == ConnectionStatus.INIT_CRYPTO_PHASE:
                    self.update_log("Settings completed successfully\n")
                    security = True

                    # send start flash mode message
                    msg = bytearray()
                    msg += bytearray.fromhex("bc")
                    msg += selected_app_update.to_bytes(1, byteorder='big')
                    msg += num_chunks.to_bytes(2, byteorder='big')
                    checksum = (msg[-2] + msg[-1]) & 0xFF
                    msg += checksum.to_bytes(1, byteorder='big')

                    # send pck
                    await client.write_gatt_char(TNS_UUID, msg, response=False)

                    # wait for ACK
                    while not ack_rcv:
                        await asyncio.sleep(0.005)
                        pass

                    ack_rcv = False

                    if not is_ack:
                        connection_status = ConnectionStatus.CLOSING_PHASE
                        QMessageBox.information(self, "FW Upload Request",
                                                "Firmware upload cannot start, the request has been denied by the target device")
                    else:
                        await asyncio.sleep(0.005)
                        connection_status = ConnectionStatus.SENDING_CRYPTO_DATA

                elif connection_status == ConnectionStatus.SENDING_DATA:  # send new FW
                    cont_chunck = 0
                    while cont_chunck < num_chunks and connection_status == ConnectionStatus.SENDING_DATA:
                        chunk = chunks[cont_chunck].ljust(16, b"0x00"[0:1])

                        msg = bytearray()
                        msg += bytearray.fromhex("bb")
                        msg += cont_chunck.to_bytes(2, byteorder='big')
                        msg += chunk
                        checksum = sum(msg[-len(chunk):]) & 0xFF
                        msg += checksum.to_bytes(1, byteorder='big')

                        await client.write_gatt_char(TNS_UUID, msg, response=False)
                        # print("Sent message")
                        cont_chunck += 1
                        print(f"Sent {cont_chunck}")

                        if cont_chunck % 10 == 0 or cont_chunck == num_chunks:
                            # wait for ACK
                            while not ack_rcv:
                                await asyncio.sleep(0.005)
                                pass

                            if not is_ack:
                                cont_chunck -= 10  # send again the last 10 chuncks of FW
                                nack_loop += 1

                                if nack_loop == 5:
                                    ret = QMessageBox.warning(self, 'NAK LOOP', "You want to try again ?", QMessageBox.Ignore , QMessageBox.Abort)

                                    if ret == QMessageBox.Ignore:
                                        nack_loop = 0
                                    else:
                                        connection_status = ConnectionStatus.CLOSING_PHASE
                            else:
                                self.update_progress((cont_chunck / num_chunks) * 100)
                                nack_loop = 0

                            ack_rcv = False
                        else:
                            await asyncio.sleep(0.02)  # time to the board to process the received data

                    connection_status = ConnectionStatus.CLOSING_PHASE

                elif connection_status == ConnectionStatus.SENDING_CRYPTO_DATA:
                    cont_chunck = 0
                    while cont_chunck < num_chunks and connection_status == ConnectionStatus.SENDING_CRYPTO_DATA:
                        chunk = chunks[cont_chunck].ljust(15, b"0x00"[0:1])
                        cont_chunck_bytes = (cont_chunck % 16).to_bytes(1, byteorder="big")

                        msg = bytearray()
                        msg.append((0xc << 4) | (cont_chunck_bytes[0] & 0x0f))

                        iv[(rand_IV[0] >> 4) % len(iv)] = (iv[(rand_IV[0] >> 4) % len(iv)] + (
                                rand_IV[0] & 0x000F)) % 256
                        iv[(rand_IV[1] >> 4) % len(iv)] = (iv[(rand_IV[1] >> 4) % len(iv)] + (
                                rand_IV[1] & 0x000F)) % 256
                        iv[(rand_IV[2] >> 4) % len(iv)] = (iv[(rand_IV[2] >> 4) % len(iv)] + (
                                rand_IV[2] & 0x000F)) % 256

                        # print(f"{rand_IV[0]}, {(rand_IV[0] >> 4) % len(iv)}, {(rand_IV[0] & 0x000F) } e {iv[(rand_IV[0] >> 4) % len(iv)]}")
                        # print(f"{rand_IV[1]}, {(rand_IV[1] >> 4) % len(iv)}, {(rand_IV[1] & 0x000F) } e {iv[(rand_IV[1] >> 4) % len(iv)]}")
                        # print(f"{rand_IV[2]}, {(rand_IV[2] >> 4) % len(iv)}, {(rand_IV[2] & 0x000F) } e {iv[(rand_IV[2] >> 4) % len(iv)]}")

                        ciphertext, tag = encrypt_message(chunk, msg[0:2], iv)
                        msg += ciphertext
                        msg += tag

                        await client.write_gatt_char(TNS_UUID, msg, response=False)
                        print("Sent message")
                        cont_chunck += 1

                        if cont_chunck % 16 == 0 or cont_chunck == num_chunks:
                            # wait for ACK
                            while msg_tampered or not ack_rcv:
                                if msg_tampered:
                                    nak_msg = bytearray()
                                    nak_msg += bytearray.fromhex("01")
                                    await client.write_gatt_char(TNS_UUID, nak_msg, response=False) # send NAK
                                    msg_tampered = False
                                    ack_rcv = False

                                while not ack_rcv:
                                    await asyncio.sleep(0.003)


                            if not is_ack:
                                print("not ack")
                                cont_chunck -= 16  # send again the last 10 chuncks of FW
                                nack_loop += 1

                                if nack_loop == 5:
                                    ret = QMessageBox.warning(self, 'NAK LOOP', "You want to try again ?",
                                                              QMessageBox.Ignore, QMessageBox.Abort)

                                    if ret == QMessageBox.Ignore:
                                        nack_loop = 0
                                    else:
                                        connection_status = ConnectionStatus.CLOSING_PHASE
                            else:
                                self.update_progress((cont_chunck / num_chunks) * 100)
                                nack_loop = 0

                            ack_rcv = False

                        else:
                            await asyncio.sleep(0.008)  # time to the board to process the received data

                    connection_status = ConnectionStatus.CLOSING_PHASE

            end_t = time.time() - start_t
            print(end_t)
            self.update_log(f"Total time : {end_t}\n")
            # FW sent, now I can end the connection
            await client.stop_notify(RCV_UUID)
            await client.disconnect()
            connection_status = ConnectionStatus.IDLE
            self.update_log("Disconnected\n")
        self.done_signal.emit()
        self.upload_finished()
        restore_crypto_utils()

class AsyncHelper(QObject):
    class ReenterQtObject(QObject):
        """ This is a QObject to which an event will be posted, allowing
            asyncio to resume when the event is handled. event.fn() is
            the next entry point of the asyncio event loop. """

        def event(self, event):
            if event.type() == QEvent.Type.User + 1:
                event.fn()
                return True
            return False

    class ReenterQtEvent(QEvent):
        """ This is the QEvent that will be handled by the ReenterQtObject.
            self.fn is the next entry point of the asyncio event loop. """

        def __init__(self, fn):
            super().__init__(QEvent.Type(QEvent.Type.User + 1))
            self.fn = fn

    def __init__(self, worker, entry):
        super().__init__()
        self.reenter_qt = self.ReenterQtObject()
        self.entry = entry
        self.loop = asyncio.new_event_loop()
        self.done = False

        self.worker = worker
        if hasattr(self.worker, "start_signal") and isinstance(self.worker.start_signal, Signal):
            self.worker.start_signal.connect(self.on_worker_started)
        if hasattr(self.worker, "done_signal") and isinstance(self.worker.done_signal, Signal):
            self.worker.done_signal.connect(self.on_worker_done)

    @Slot()
    def on_worker_started(self):
        """ To use asyncio and Qt together, one must run the asyncio
            event loop as a "guest" inside the Qt "host" event loop. """
        if not self.entry:
            raise Exception("No entry point for the asyncio event loop was set.")
        asyncio.set_event_loop(self.loop)
        self.loop.create_task(self.entry())
        self.loop.call_soon(self.next_guest_run_schedule)
        self.done = False  # Set this explicitly as we might want to restart the guest run.
        self.loop.run_forever()

    @Slot()
    def on_worker_done(self):
        """ When all our current asyncio tasks are finished, we must end
            the "guest run" lest we enter a quasi idle loop of switching
            back and forth between the asyncio and Qt loops. We can
            launch a new guest run by calling launch_guest_run() again. """
        self.done = True

    def continue_loop(self):
        """ This function is called by an event posted to the Qt event
            loop to continue the asyncio event loop. """
        if not self.done:
            self.loop.call_soon(self.next_guest_run_schedule)
            self.loop.run_forever()

    def next_guest_run_schedule(self):
        """ This function serves to pause and re-schedule the guest
            (asyncio) event loop inside the host (Qt) event loop. It is
            registered in asyncio as a callback to be called at the next
            iteration of the event loop. When this function runs, it
            first stops the asyncio event loop, then by posting an event
            on the Qt event loop, it both relinquishes to Qt's event
            loop and also schedules the asyncio event loop to run again.
            Upon handling this event, a function will be called that
            resumes the asyncio event loop. """
        self.loop.stop()
        QApplication.postEvent(self.reenter_qt, self.ReenterQtEvent(self.continue_loop))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    main_window = MainWindow()
    async_helper = AsyncHelper(main_window, main_window.send_new_fw)

    main_window.show()

    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app.exec()
