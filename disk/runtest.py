import itertools,subprocess
import sys
import os.path
from time import gmtime, strftime

#./disktest.x nthreads access-type block-size data-size-per-thread file-path rwmode
exefile = "./disktest.x"
nthreads = ["1","2"]
actype = ["0", "1"]
blocksize = [str(1024*1024*1024), str(1024*1024), "1024", "1"] #, str(1024*1024*1024)]
dataperthread = [str(1024*1024*1024)]
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
    header = "nthreads dataperthread fpath    filesize totaltime bandwidth latency blocksize blockcnt rwmode actype ddbandwidth"
    resultf.write(header + "\n")
    print "started running",

    for rep in range(1):
        for para in paralist:
            para = list(para)
            pathstr = "/panfs/scratch2/vol1/jun/" + ''.join(str(e)+"." for e in para[0:4]) + 'dat'
            para[4] = pathstr
            
            # clear cache
            if os.path.exists(pathstr):
                print "clearing cache of " + pathstr
                clear_proc = subprocess.Popen(["./clearfilecache.x",
                                   pathstr],
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
                clear_proc.wait()

            mycmd = [exefile] + list(para)
            print mycmd
            proc = subprocess.Popen(mycmd,
                           stdout=subprocess.PIPE,
                           stderr=logf)
            proc.wait()
            
            #dd if=/dev/urandom of=file.txt bs=2048 count=10
            bsize = para[2]
            bcnt = str(int(para[0])*int(para[3])/int(para[2]))
            ddpathstr = pathstr+".dd"
            if para[-1] == "0":
                # read
                ddcmd = ["dd", "if="+ddpathstr, "of=/dev/null",
                              "bs="+bsize, "count="+bcnt]
            else:
                # write
                ddcmd = ["dd", "if=/dev/urandom",
                               "of="+ddpathstr,
                               "bs="+bsize,
                               "count="+bcnt]
            print ddcmd
            dd_proc = subprocess.Popen(ddcmd,
                                        stderr=subprocess.PIPE,
                                        stdout=subprocess.PIPE)
            dd_proc.wait()
            ddbandwidth = ""
            for line in dd_proc.stderr:
                print "dd ", line
                if "copied" in line:
                    ddperf = line.split()
                    print ddperf
                    ddbandwidth = float(ddperf[0])/(float(ddperf[5])*1024.0*1024.0)

            for line in proc.stdout:
                if 'GREPMARKER' in line:
                    parastr = ''.join(str(e)+" " for e in [para[0], para[3], para[4]])
                    perf = line.split()
                    perfstr = ''.join(str(e)+" " for e in perf[0:-1])
                    print parastr, perfstr, ddbandwidth
                    resultf.write( parastr + perfstr + str(ddbandwidth) + "\n" ) 
                    sys.stdout.flush()
    
    print "see the results in the latest *.result file"
    logf.close()
    resultf.close()

