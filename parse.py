import os
import sys
import numpy as np

#file = str(sys.argv[1])
s_file = 0
num = 0

def parse_file(head, number, reqtype, dir):
    res_avg = 0
    tail = 0
    tot = []
    length = 0
    res_min = float("inf")
    res_max = 0
    tot_sent = 0
    tot_made = 0
    tot_drop = 0
    tot_recv = 0
    tot_miss = 0
    tmp_avg = 0
    rtts = 0

    for c in range(number):
        idx = head + c
        filename = dir+"/mcb_"+str(idx)
        with open(filename, "r") as f:
            for line in f:
                if line.startswith(reqtype+":"):
                    temp = int(line.split()[1].strip())
                    #res_min = min(res_min, temp);
                    #res_max = max(res_max, temp);
                    #res_avg += temp
                    length += 1
                    tot.append(temp)
                elif line.startswith(reqtype+" RTT min/avg/max"):
                    all = line.split(':')[1].strip().split('/')
                    res_min = min(res_min, int(all[0]))
                    res_max = max(res_max, int(all[2].split()[0]))
                    tmp_avg = int(all[1])
                elif line.startswith(reqtype+" sent"):
                    temp = int(line.split(None)[-1].strip())
                    tot_sent += temp
                elif line.startswith(reqtype+" Deadline made"):
                    temp = int(line.split(None)[-1].strip())
                    tot_made += temp
                elif line.startswith(reqtype+" Measured RTTs"):
                    rtts = int(line.split(None)[-1].strip())
                    tot_recv += rtts
            res_avg += (tmp_avg * rtts)
    tot.sort()
    res_avg = res_avg / tot_recv
    tail = int(length*0.99)
    tot_drop = tot_sent-tot_recv
    tot_miss = tot_sent-tot_made

    print("[{}]parsed file:                {} - {}".format(reqtype, "mcb_"+str(head), "mcb_"+str(head+number-1)))
    print("[{}]tail latecny:               {} us".format(reqtype, tot[tail] if length else 0))
    print("[{}]avgerage latency:           {:.2f} us".format(reqtype, res_avg if length else 0))
    print("[{}]MIN latency:                {} us".format(reqtype, res_min))
    print("[{}]MAX latency:                {} us".format(reqtype, res_max))
    print("[{}]number of RTTs measured:    {}".format(reqtype, tot_recv))
    print("[{}]Total request sent:         {}".format(reqtype, tot_sent))
    print("[{}]number of requests dropped: {}".format(reqtype, tot_drop))
    '''
    print("[{}]number of deadline made:    {}".format(reqtype, tot_made))
    print("[{}]% of deadline missed:       {:.2f}".format(reqtype, (tot_miss)*100/tot_sent))
    '''
    print("-----------------------------------------------------------")

if __name__ == "__main__":
    s_file = 11211
    nb = int(sys.argv[1])
    dir = sys.argv[2]
    parse_file(s_file, nb, "set", dir)
    parse_file(s_file, nb, "get", dir)

