import subprocess
import argparse
import time

cur_core = 0
nb_cores = 1
server = "10.10.1.2"
t = 1
k = 1
z = 135
d = 10
#rate_base = 10;
flow_per_core = 50
rate = 500000
#rate=[]

class client(object):
    def __init__(self, tcp_port, udp_port, core):
        self.tcp_port = tcp_port
        self.udp_port = udp_port

#       index = ((client_port-11211)%flow_per_core)
#       self.rate = rate[index]

        self.args = ["taskset"]
        self.args.extend(["-c", "0-7"])
        self.args.extend(["memcached"])
        self.args.extend(["--listen",str(server)])
        self.args.extend(["--port",str(tcp_port)])
        self.args.extend(["--udp-port",str(udp_port)])
        self.args.extend(["--threads",str(1)])
        self.args.extend(["--protocol","auto"])
        self.args.extend(["--memory-limit",str(16)])
        self.args.extend(["--extended","no_lru_crawler,no_lru_maintainer,no_hashexpand,no_slab_reassign,no_slab_automove"])

        self.log_file_path, self.log_file = self.create_log()
        print(self.args)
        
        self.p = subprocess.Popen(self.args, stdout=self.log_file)
        self.log_file.close()

    def stop(self):
        while self.process.poll() is None:
            time.sleep(1)
        self.log_file_path.close()

    def create_log(self):
        file_path = "slogs/server_{}".format(self.udp_port)
        f = open(file_path, "w+")
        f.write("ab arguments: {}\n\nSTDOUT:\n".format(str(self.args)))
        f.flush()
        return file_path, f

def increment_core_num(core):
    global cur_core
    p = core + cur_core
    cur_core = (cur_core + 1) % nb_cores
    return p

def start_clients(nb_nodes, tcp_port, udp_port):
    client_list=[]
    for _ in range(nb_nodes):
        nc = increment_core_num(0)
        client_list.append(client(tcp_port, udp_port, nc))

        if tcp_port > 0 and udp_port > 0:
            tcp_port += 1
            udp_port += 1
    return client_list

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--nb_nodes", help="number of nodes", type=int)
    parser.add_argument("--nb_cores", help="number of cores", type=int)
    parser.add_argument("--tcp_port", help="tcp_port", type=int)
    parser.add_argument("--udp_port", help="udp_port", type=int)
    return parser.parse_args()

if __name__ == '__main__':

    args = parse_args()
    nb_nodes = args.nb_nodes if args.nb_nodes else 1
    nb_cores = args.nb_cores if args.nb_cores else 16
    tcp_port = args.tcp_port if args.tcp_port else 11220
    udp_port = args.udp_port if args.udp_port else 11211

    client_list = start_clients(nb_nodes, tcp_port, udp_port)

