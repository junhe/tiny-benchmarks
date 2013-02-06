import subprocess

serverip = "rrz023"
clientip = "rrz022"
udpflag=" -u "

subprocess.call("srun -n1 -N1 -w " + serverip + " pkill iperf", shell=True)

ser_cmd = "srun -n1 -N1 -w " + serverip + " iperf -s" + udpflag
ser_proc = subprocess.Popen(ser_cmd, shell=True)

cli_cmd = "srun -n1 -N1 -w " + clientip + " iperf -c " + serverip + udpflag
print "starting client..."
cli_proc = subprocess.Popen(cli_cmd, shell=True)
cli_proc.wait()

