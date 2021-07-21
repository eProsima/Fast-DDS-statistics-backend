# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys, os, subprocess, glob, threading, time, queue

script_dir = os.path.dirname(os.path.realpath(__file__))

publisher_command = os.environ.get("DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN")
if not publisher_command:
    publisher_files = glob.glob(os.path.join(script_dir, "**/DDSSimpleCommunicationPublisher*"), recursive=True)
    pf = iter(publisher_files)
    publisher_command = next(pf, None)
    while publisher_command and (not os.path.isfile(publisher_command) or not os.access(publisher_command,
        os.X_OK)):
        publisher_command = next(pf, None)

subscriber_command = os.environ.get("DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN")
if not subscriber_command:
    subscriber_files = glob.glob(os.path.join(script_dir, "**/DDSSimpleCommunicationSubscriber*"), recursive=True)
    pf = iter(subscriber_files)
    subscriber_command = next(pf, None)
    while subscriber_command and (not os.path.isfile(subscriber_command) or not os.access(subscriber_command,
        os.X_OK)):
        subscriber_command = next(pf, None)

monitor_command = os.environ.get("DDS_SIMPLE_COMMUNICATION_MONITOR_BIN")
if not monitor_command:
    monitor_files = glob.glob(os.path.join(script_dir, "**/DDSSimpleCommunicationMonitor*"), recursive=True)
    pf = iter(monitor_files)
    monitor_command = next(pf, None)
    while monitor_command and (not os.path.isfile(monitor_command) or not os.access(monitor_command,
        os.X_OK)):
        monitor_command = next(pf, None)

extra_pub_arg = os.environ.get("EXTRA_PUB_ARG")
if extra_pub_arg:
    extra_pub_args = extra_pub_arg.split()
else:
    extra_pub_args = []

real_xml_file_pub = None
real_xml_file_sub = None
xml_file = os.environ.get("XML_FILE")
if xml_file:
    real_xml_file_pub = os.path.join(script_dir, xml_file)
    real_xml_file_sub = os.path.join(script_dir, xml_file)
else:
    xml_file_pub = os.environ.get("XML_FILE_PUB")
    if xml_file_pub:
        real_xml_file_pub = os.path.join(script_dir, xml_file_pub)
    xml_file_sub = os.environ.get("XML_FILE_SUB")
    if xml_file_sub:
        real_xml_file_sub = os.path.join(script_dir, xml_file_sub)

# Thread that read the monitor_proc stdout and push it into a queue
def output_reader(proc, outq):
    for line in iter(proc.stdout.readline, b''):
        outq.put(line.decode('utf-8'))

def communication(monitor_proc, pid):
    """A"""
    outq = queue.Queue()
    t = threading.Thread(target=output_reader, args=(monitor_proc,outq))
    t.start()

    run = True
    try:
        time.sleep(0.2)
        
        while run:
            try:
                line = outq.get(block=False).rstrip()

                sys.stdout.flush()

                # Monitor initialized
                if (line == ("Init Monitor_" + pid)):
                    print("___" + pid + "___Init Monitor___")
                    sys.stdout.flush()

                    print("___" + pid + "___Creating subscriber___")
                    sys.stdout.flush()
                    subscriber1_proc = subprocess.Popen([subscriber_command, "--seed", pid]
                        + (["--xmlfile", real_xml_file_sub] if real_xml_file_sub else []))
                    
                    print("___" + pid + "___Creating publisher___")
                    sys.stdout.flush()
                    publisher_proc1 = subprocess.Popen([publisher_command, "--seed", pid, "--wait", "1"]
                        + (["--xmlfile", real_xml_file_pub] if real_xml_file_pub else [])
                        + extra_pub_args)

                    print("___" + pid + "___subscriber1 communicate...___")
                    sys.stdout.flush()
                    publisher_proc1.communicate()

                    print("___" + pid + "___publisher1 communicate...___")
                    sys.stdout.flush()
                    subscriber1_proc.communicate()

                elif (line == ("Stop Monitor_" + pid)):
                    print("___" + pid + "___Stop Monitor___")
                    sys.stdout.flush()
                    run = False

                else:
                    print("___" + pid + '_ ' + line)
                    sys.stdout.flush()

            except queue.Empty:
                print("___" + pid + '_ could not get line from queue')
                sys.stdout.flush()

            time.sleep(0.1)
    finally:
        monitor_proc.terminate()
        try:
            monitor_proc.wait(timeout=0.2)
            print("___" + pid + '== subprocess exited with rc =', monitor_proc.returncode)
        except subprocess.TimeoutExpired:
            print("___" + pid + '_ subprocess did not terminate in time')

    while outq.empty() != True:
        line = outq.get(block=False).rstrip()
        print("___" + pid + '_ ' + line)
        sys.stdout.flush()

    t.join()

monitor_proc_0 = subprocess.Popen([monitor_command, "--seed", str(os.getpid())],
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

monitor_proc_1 = subprocess.Popen([monitor_command, "--seed", str(os.getpid() + 1)],
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

t_0 = threading.Thread(target=communication, args=(monitor_proc_0,str(os.getpid())))
t_1 = threading.Thread(target=communication, args=(monitor_proc_1,str(os.getpid() + 1)))

t_0.start()
t_1.start()

t_0.join()
t_1.join()

retvalue = monitor_proc_0.returncode
if retvalue == 0:
    retvalue = monitor_proc_1.returncode

sys.exit(retvalue)
