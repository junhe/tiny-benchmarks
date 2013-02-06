import itertools,subprocess
import sys
from time import gmtime, strftime

#./server.x nthreads protocol-type buffer-size portno
ser_exefile = "./server.x"
ser_protocoltype = ["0", "1"]
ser_buffersize = ["1", "1024", str(1024*64)]
ser_portno = ["10000"]

ser_parameters = [ser_protocoltype, ser_buffersize, ser_portno]
ser_paralist = list(itertools.product(*ser_parameters))


jobid = strftime("%Y-%m-%d-%H-%M-%S", gmtime())
logname = jobid + ".log"
resultname = jobid + ".result"

if __name__ == '__main__':
    logf = open(logname, 'a')
    resultf = open(resultname, 'a')
    #printf("Total-Data-Size Total-Time(second) Bandwidth(MB/s) Latency(ms)"
    #           " Protocol\n");
    header = "nthreads buffersize dataperthread    totalsize totaltime bandwidth latency protocal"
    resultf.write(header + "\n")
    print header
    
    # clean up
    killer = subprocess.Popen(["pkill", "server.x"])
    killer.wait()

    for rep in range(1):
        for ser_para in ser_paralist:
            ser_para = list(ser_para)
            #print ser_para,

            mycmd = [ser_exefile] + list(ser_para)

            ser_proc = subprocess.Popen(mycmd)
                           #stdout=subprocess.PIPE,
                           #stderr=logf)

            # now run the client
            #./client.x nthreads protocol-type buffer-size data-size-per-thread ip portno 
            cli_exefile = "./client.x"
            cli_nthreads = ["1", "2"]
            cli_protocoltype = ["0", "1"]
            cli_buffersize = [ser_para[1]]
            cli_dataperthread = [str(1024*1024)]
            cli_ip = ["localhost"]
            cli_portno = ser_portno

            cli_parameters = [cli_nthreads, cli_protocoltype, cli_buffersize,
                              cli_dataperthread, cli_ip, cli_portno]
            cli_paralist = list(itertools.product(*cli_parameters))

            for cli_para in cli_paralist:
                cli_cmd = [cli_exefile] + list(cli_para)
                cli_proc = subprocess.Popen(cli_cmd,
                                            stdout = subprocess.PIPE,
                                            stderr = logf)
                cli_proc.wait() 
                for line in cli_proc.stdout:
                    #print line,
                    if 'GREPMARKER' in line:
                        parastr = ''.join(str(e)+" " for e in [cli_para[0], cli_para[2], cli_para[3]])
                        perf = line.split()
                        perfstr = ''.join(str(e)+" " for e in perf[0:-1])
                        print parastr, perfstr
                        resultf.write( parastr + perfstr + "\n" ) 
                        sys.stdout.flush()

            ser_proc.terminate()
            killer = subprocess.Popen(["pkill", "server.x"])
            killer.wait()
        
    logf.close()
    resultf.close()
