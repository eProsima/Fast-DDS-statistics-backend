# Helloworld Example

In this example we set up a monitor which measures and prints the latency and throughput of the communication between a
publisher and subscriber.

## Execution instructions

To launch this test open three different consoles:

In the first one launch: ./HelloWorldExample publisher (or HelloWorldExample.exe publisher on windows).
In the second one: ./HelloWorldExample subscriber (or HelloWorldExample.exe subscriber on windows).
In the third one: ./HelloWorldExample monitor (or HelloWorldExample.exe monitor on windows).

## Arguments

First argument is `publisher`, `subscriber` or `monitor` and then the rest of arguments are read unordered

```sh
Usage: HelloWorldExample <publisher|subscriber|monitor>

General options:
  -h       --help            Produce help message.

Publisher options:
  -d <id>  --domain=<id>     DDS domain ID (Default: 0).
  -s <num> --samples=<num>   Number of samples to send (Default: 0 => infinite
                             samples).
  -i <num> --interval=<num>  Time between samples in milliseconds (Default:
                             100).

Subscriber options:
  -d <id>  --domain=<id>     DDS domain ID (Default: 0).
  -s <num> --samples=<num>   Number of samples to wait for (Default: 0 =>
                             infinite samples).

Monitor options:
  -d <id>  --domain=<id>     DDS domain ID (Default: 0).
  -b <num> --bins=<num>      Number of bins in which a time interval is divided
                             (Default: 1) (0 => no mean calculation, return raw
                             data).
  -t <num> --time=<num>      Duration in seconds of each time frame (Default:
                             5).
```
