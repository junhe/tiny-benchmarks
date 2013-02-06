import itertools,subprocess
import sys
from time import gmtime, strftime

#nthreads access-type block-size memory-size-per-thread
exefile = "./memtest.x"
nthreads = ["1","2"]
actype = ["0", "1"]
blocksize = ["1","1024", str(1024*1024)]
memperthread = [str(1024*1024)]

parameters = [nthreads, actype, blocksize, memperthread]

paralist = list(itertools.product(*parameters))

jobid = strftime("%Y-%m-%d-%H-%M-%S", gmtime())
logname = jobid + ".log"
resultname = jobid + ".result"

if __name__ == '__main__':
    logf = open(logname, 'a')
    resultf = open(resultname, 'a')
    #Copied-Size Total-Time(second) Bandwidth(MB/s) Latency(ms) Block-Size Block-Count Access-Type
    header = "nthreads memperthread totalsize totaltime bandwidth latency blocksize blockcnt actype"
    resultf.write(header + "\n")
    print header

    for rep in range(10):
        for para in paralist:
            mycmd = [exefile] + list(para)
            proc = subprocess.Popen(mycmd,
                           stdout=subprocess.PIPE,
                           stderr=logf)
            proc.wait()
            for line in proc.stdout:
                if 'GREPMARKER' in line:
                    para = list(para)
                    parastr = ''.join(str(e)+" " for e in [para[0], para[3]])
                    perf = line.split()
                    perfstr = ''.join(str(e)+" " for e in perf[0:7])
                    print parastr, perfstr
                    resultf.write( parastr + perfstr + "\n" ) 
                    sys.stdout.flush()
    
    logf.close()
    resultf.close()

