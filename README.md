# netflow-v5-collector-zephyr

A Zephyr RTOS application that listens for **NetFlow v5** packets over UDP and parses flow data in real time.

## Getting started

```bash
west init -m https://github.com/pkoscik/netflow-v5-collector-zephyr --mr main netflow-v5-collector-zephyr
cd netflow-v5-collector-zephyr
west update
```

> [!NOTE]  
> Remember to configure the network interface IP address in your `prj.conf` (or board overlay files), and ensure the NetFlow UDP port (default 2055) is set correctly in `main.c` if you change it.

```bash
west build -b <board_name> app
west flash
```

## Output example

```text
*** Booting Zephyr OS build v4.2.0-rc1 ***
[00:00:01.552,000] <inf> phy_mii: PHY (0) Link speed 100 Mb, full duplex
[00:00:01.558,000] <inf> netflow_listener: Listening on UDP port 2055
[00:00:02.753,000] <inf> netflow_listener: NetFlow v5 packet: 9 flows from 192.168.1.1
[00:00:02.753,000] <dbg> netflow_listener: netflow_listener_thread: SysUptime=14912570, UnixSecs=1750904051, FlowSeq=52694
[00:00:02.753,000] <inf> netflow_listener: Flow 0: 216.239.36.158:443 → 10.0.0.3:48564 proto=6 bytes=535 packets=5
[00:00:02.753,000] <inf> netflow_listener: Flow 1: 192.168.1.99:46506 → 109.173.188.64:13231 proto=17 bytes=9000 packets=22
[00:00:02.753,000] <inf> netflow_listener: Flow 2: 10.0.0.3:48564 → 216.239.36.158:443 proto=6 bytes=284 packets=5
[00:00:02.753,000] <inf> netflow_listener: Flow 3: 192.168.1.203:55776 → 20.33.36.94:443 proto=6 bytes=137 packets=2
[00:00:02.753,000] <inf> netflow_listener: Flow 4: 20.33.36.94:443 → 192.168.1.203:55776 proto=6 bytes=97 packets=1
[00:00:02.753,000] <inf> netflow_listener: Flow 5: 109.173.188.64:36257 → 62.21.99.94:53 proto=17 bytes=74 packets=1
[00:00:02.753,000] <inf> netflow_listener: Flow 6: 62.21.99.94:53 → 109.173.188.64:36257 proto=17 bytes=90 packets=1
[00:00:02.753,000] <inf> netflow_listener: Flow 7: 192.168.1.202:62834 → 20.33.36.89:443 proto=6 bytes=137 packets=2
[00:00:02.753,000] <inf> netflow_listener: Flow 8: 20.33.36.89:443 → 192.168.1.202:62834 proto=6 bytes=97 packets=1
```
