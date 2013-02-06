import itertools,subprocess
import sys
from time import gmtime, strftime

exefile = "./cputest.x"
nthreads = ["1","2","4","8"]
optype = ["flops", "iops"]
nloops = ["100000000"]

parameters = [nthreads, optype, nloops]

paralist = list(itertools.product(*parameters))

jobid = strftime("%Y-%m-%d-%H-%M-%S", gmtime())
logname = jobid + ".log"
resultname = jobid + ".result"

if __name__ == '__main__':
    logf = open(logname, 'a')
    resultf = open(resultname, 'a')
    header = "nthreads optype nloops totaltime opps"
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
                    parastr = ''.join(str(e)+" " for e in list(para))
                    perf = line.split()[1:3]
                    perfstr = ''.join(str(e)+" " for e in perf)
                    print parastr, perfstr
                    resultf.write( parastr + perfstr + "\n" ) 
                    sys.stdout.flush()
        
    
    logf.close()
    resultf.close()

