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
assert publisher_command
subscriber_command = os.environ.get("DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN")
if not subscriber_command:
    subscriber_files = glob.glob(os.path.join(script_dir, "**/DDSSimpleCommunicationSubscriber*"), recursive=True)
    pf = iter(subscriber_files)
    subscriber_command = next(pf, None)
    while subscriber_command and (not os.path.isfile(subscriber_command) or not os.access(subscriber_command,
        os.X_OK)):
        subscriber_command = next(pf, None)
assert subscriber_command

monitor_command = os.environ.get("DDS_SIMPLE_COMMUNICATION_MONITOR_BIN")

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


monitor_proc = subprocess.Popen([monitor_command, "--seed", str(os.getpid())],
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)

outq = queue.Queue()
t = threading.Thread(target=output_reader, args=(monitor_proc,outq))
t.start()

run = True

try:
    time.sleep(0.2)
    
    while run:
        try:
            line = outq.get(block=False).rstrip()

            print(line)
            sys.stdout.flush()

            # Monitor initialized
            if (line == "Init Monitor"):
                print("___Init Monitor___")
                sys.stdout.flush()

                print("___Creating subscriber1___")
                sys.stdout.flush()
                subscriber1_proc = subprocess.Popen([subscriber_command, "--seed", str(os.getpid())]
                    + (["--xmlfile", real_xml_file_sub] if real_xml_file_sub else []))
                
                print("___Creating publisher1___")
                sys.stdout.flush()
                publisher_proc1 = subprocess.Popen([publisher_command, "--seed", str(os.getpid())]
                    + (["--xmlfile", real_xml_file_pub] if real_xml_file_pub else [])
                    + extra_pub_args)

                print("___subscriber1 communicate...___")
                sys.stdout.flush()
                subscriber1_proc.communicate()

                print("___publisher1 kill___")
                sys.stdout.flush()
                publisher_proc1.communicate()

                print("___Stop Monitor___")
                sys.stdout.flush()
                run = False

        except queue.Empty:
            print('could not get line from queue')
            sys.stdout.flush()

        time.sleep(0.1)
finally:
    monitor_proc.terminate()
    try:
        monitor_proc.wait(timeout=0.2)
        print('== subprocess exited with rc =', monitor_proc.returncode)
    except subprocess.TimeoutExpired:
        print('subprocess did not terminate in time')

while outq.empty() != True:
    line = outq.get(block=False).rstrip()
    print(line)
    sys.stdout.flush()


t.join()

sys.exit(monitor_proc.returncode)
