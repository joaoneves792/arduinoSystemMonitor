import subprocess
import serial
import time
import re
from struct import pack
    
core_0_regex = re.compile(b'Core 0:       \+(\d\d)\.\d')
core_1_regex = re.compile(b'Core 1:       \+(\d\d)\.\d')
core_2_regex = re.compile(b'Core 2:       \+(\d\d)\.\d')
core_3_regex = re.compile(b'Core 3:       \+(\d\d)\.\d')

gpu_regex = re.compile(b'\| \d+\%\ +(\d\d)C\ +P(\d)(?:\d)?\ +N/A')

cpu_clock_regex = re.compile(b'cpu MHz\s+: (\d+)\.\d+')

def get_cpu_temps():
    data_packet = ""

    cpu_temp_proc = subprocess.Popen("sensors" , stdout=subprocess.PIPE)
    output = cpu_temp_proc.stdout.read()
    data_packet += core_0_regex.findall(output)[0].decode('utf-8')
    data_packet += core_1_regex.findall(output)[0].decode('utf-8')
    data_packet += core_2_regex.findall(output)[0].decode('utf-8')
    data_packet += core_3_regex.findall(output)[0].decode('utf-8')
    
    return data_packet

def get_cpu_clock():
    cpu_clock_proc = subprocess.Popen(("cat", "/proc/cpuinfo"), stdout=subprocess.PIPE)
    output = cpu_clock_proc.stdout.read()
    clocks = cpu_clock_regex.findall(output)
    avgClock = 0
    for clock in clocks:
        intClock = int(clock.decode('utf-8')) 
        avgClock += intClock

    avgClock /= 4

    if avgClock > 2450:
        return "0"
    else:
        return "1"


def get_gpu_info():
    data_packet = ""
    gpu_info_proc = subprocess.Popen("nvidia-smi", stdout=subprocess.PIPE)
    output = gpu_info_proc.stdout.read()
    data_packet += gpu_regex.findall(output)[0][0].decode('utf-8')
    data_packet += gpu_regex.findall(output)[0][1].decode('utf-8')

    return data_packet

def connect():
    connected = False
    while not connected:
        try:
            s = serial.Serial('/dev/ttyACM0', 9600, timeout=None)
            connected = True
        except serial.SerialException as e:
            try:
                s = serial.Serial('/dev/ttyACM1', 9600, timeout=None)
                connected = True
            except serial.SerialException as e:
                time.sleep(1)

    return s

ser = connect()

while True:
    time.sleep(1)
    data_packet = get_cpu_temps() + get_gpu_info() + get_cpu_clock()
    try:
        ser.write(data_packet.encode('ascii'))
    except (TypeError, serial.SerialException) as e:
        ser.close()
        ser = connect()

