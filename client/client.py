import socket
import struct
import sys
import threading
import time

import numpy as np
import matplotlib.pyplot as plt

arr_timestamp = [0]
arr_macd = [0]
arr_signal = [0]

lock = threading.Lock()

def drawing():
    fig = plt.figure(figsize=(16, 9), dpi=64)
    plt.grid()
    ax = fig.add_subplot(1, 1, 1)
    macd_line, = ax.plot(arr_timestamp, arr_macd, color='#ff4500')
    signal_line, = ax.plot(arr_timestamp, arr_signal, color='#4169e1')

    arr_macd_num = 1
    is_recv = False

    x_time = 0
    while True:
        global lock
        lock.acquire()

        if is_recv == False and len(arr_macd) <= 1:
            lock.release()
            time.sleep(1)
            continue
        else:
            if is_recv == False:
                is_recv = True
                del arr_timestamp[0]
                del arr_macd[0]
                del arr_signal[0]
                arr_macd_num -= 1

        if (arr_macd_num > 2000):
            del arr_timestamp[0]
            del arr_macd[0]
            del arr_signal[0]
            arr_macd_num -= 1

        x_time += 1
        arr_timestamp.append(x_time)

        if len(arr_macd) == arr_macd_num:
            arr_macd.append(arr_macd[-1])
            arr_signal.append(arr_signal[-1])

        arr_macd_num = len(arr_macd)

        # 数チェック
        x_num = len(arr_timestamp)
        y1_num = len(arr_macd)
        y2_num = len(arr_signal)

        # パケットが雪崩込んだ時に x, y のデータ数がズレる
        # y < x となるので x が y に追いつくまでインクリメントする
        while x_num < y1_num:
            print("recover: ++x")    
            x_time += 1
            x_num += 1
            arr_timestamp.append(x_time)

        # パケットが間に合わなかった時に x, y のデータ数がズレる
        # y > x となるので x が y に追いつくまでインクリメントする
        while x_num > y1_num:
            print("recover: --x")    
            x_time -= 1
            x_num -= 1
            arr_timestamp.pop(0)

        macd_line.set_data(arr_timestamp, arr_macd)
        signal_line.set_data(arr_timestamp, arr_signal)

        m_min = min(arr_macd)
        m_max = max(arr_macd)
        s_min = min(arr_signal)
        s_max = max(arr_signal)

        _min, _max = 0, 0
        if m_min > s_min:
            _min = s_min
        else:
            _min = m_min

        if m_max > s_max:
            _max = m_max
        else:
            _max = s_max

        ax.set_ylim(_min - 10, _max + 10)
        ax.set_xlim(min(arr_timestamp), max(arr_timestamp))

        plt.draw()

        lock.release()
        plt.pause(5)

def recv():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('172.16.80.98', 37777))
        s.listen(1)
        while True:
            conn, addr = s.accept()
            with conn:
                while True:
                    msg = conn.recv(64)

                    if not msg:
                        break

                    global lock
                    lock.acquire()

                    macd, signal = struct.unpack("ff", msg)
                    print("--------------------------")
                    print("m", macd)
                    print("s", signal)

                    arr_macd.append(macd)
                    arr_signal.append(signal)

                    lock.release()

th = threading.Thread(target=recv)
th.start()
drawing()
th.join()
