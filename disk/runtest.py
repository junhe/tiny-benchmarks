import itertools,subprocess
import sys
from time import gmtime, strftime

#./disktest.x nthreads access-type block-size data-size-per-thread file-path rwmode
exefile = "./disktest.x"
nthreads = ["1","2"]
actype = ["0", "1"]
blocksize = ["1","1024", str(1024*1024), str(1024*1024*1024)]
dataperthread = [str(1024)]
fpath = ["PATHHOLDER"]
rwmode = ["0", "1"]

parameters = [nthreads, actype, blocksize, dataperthread, fpath, rwmode]

paralist = list(itertools.product(*parameters))
paralist = [list(e) for e in paralist]
paralist.sort(key=lambda x: (float(x[5])), reverse=True)

jobid = strftime("%Y-%m-%d-%H-%M-%S", gmtime())
logname = jobid + ".log"
resultname = jobid + ".result"

if __name__ == '__main__':
    logf = open(logname, 'a')
    resultf = open(resultname, 'a')
    #printf("    File-Size Total-Time(second) Bandwidth(MB/s) Latency(ms) Block-Size Block-Count"
    #           "   Rwmode Access-Type\n");
    header = "nthreads dataperthread fpath    filesize totaltime bandwidth latency blocksize blockcnt rwmode actype"
    resultf.write(header + "\n")
    print header

    for rep in range(10):
        for para in paralist:
            para = list(para)
            pathstr = ''.join(str(e)+"." for e in para[0:4]) + 'dat'
            para[4] = pathstr

            mycmd = [exefile] + list(para)
            proc = subprocess.Popen(mycmd,
                           stdout=subprocess.PIPE,
                           stderr=logf)
            proc.wait()
            for line in proc.stdout:
                if 'GREPMARKER' in line:
                    parastr = ''.join(str(e)+" " for e in [para[0], para[3], para[4]])
                    perf = line.split()
                    perfstr = ''.join(str(e)+" " for e in perf[0:-1])
                    print parastr, perfstr
                    resultf.write( parastr + perfstr + "\n" ) 
                    sys.stdout.flush()
    
    logf.close()
    resultf.close()

