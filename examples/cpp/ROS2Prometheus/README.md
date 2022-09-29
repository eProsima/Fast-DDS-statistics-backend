# ROS2Prometheus Example

In this example we set up a monitor which measures and prints the latency and throughput of the communication between a
ROS 2 talker and listener from the `demo_nodes_cpp` package.
This example also exports the statistics collected by the monitor to Prometheus.

## Additional dependencies

Additional dependencies are required to run this example.

* [C++ Prometheus exporter](https://github.com/rsanchez15/prometheus-cpp) (required)
* [Prometheus](https://prometheus.io/) (optional)

## Execution instructions

To launch this test open three different consoles:

* Console 1:

    ```bash
    export FASTDDS_STATISTICS="HISTORY_LATENCY_TOPIC;PUBLICATION_THROUGHPUT_TOPIC;PHYSICAL_DATA_TOPIC"
    ros2 run demo_nodes_cpp talker
    ```

* Console 2:

    ```bash
    export FASTDDS_STATISTICS="HISTORY_LATENCY_TOPIC;PUBLICATION_THROUGHPUT_TOPIC;PHYSICAL_DATA_TOPIC"
    ros2 run demo_nodes_cpp listener
    ```

* Console 3:

    ```bash
    ./ROS2Prometheus
    ```


## Arguments

```sh
Usage: ROS2Prometheus

General options:
  -h            --help              Produce help message.

Monitor options:
  -d <id>       --domain=<id>       DDS domain ID (Default: 0).
  -b <num>      --bins=<num>        Number of bins in which a time interval is divided
                                    (Default: 1) (0 => no mean calculation, return raw
                                    data).
  -t <num>      --time=<num>        Duration in seconds of each time frame
                                    (Default: 5).

Prometheus options:
  -a <IP:port>  --domain=<id>     DDS domain ID (Default: 127.0.0.1:8080).
```
