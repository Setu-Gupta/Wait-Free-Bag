import csv
from matplotlib import pyplot as plt
import sys

csv_dir = '../collect'
out_dir = './'
if len(sys.argv) == 3:
    csv_dir = sys.argv[1]
    out_dir = sys.argv[2]

ps = []
insert = []
iterate = []
extract = []
with open(csv_dir + '/weak.csv', newline='') as infile:
    reader = csv.reader(infile)
    next(reader)
    for row in reader:
        ps.append(float(row[0]))
        insert.append(float(row[2]))
        iterate.append(float(row[3]))
        extract.append(float(row[4]))

plt.figure(figsize=(9, 5))
plt.grid()
plt.title("Weak Scaling Study")
plt.xlabel("Number of threads, $P =$ scaling factor for $N$")
plt.ylabel("Speedup over blocking implementation with $N = (P \\times 2^{10})$")
plt.autoscale(axis = 'x', tight = True, enable = True)
plt.plot(ps, insert, 'ro-', label="Insertion")
plt.plot(ps, iterate, 'go-', label="Iteration")
plt.plot(ps, extract, 'bo-', label="Extraction")
plt.legend()
plt.savefig(out_dir + '/weak.svg', dpi=500)

ps = []
blocking = []
non_blocking = []
with open(csv_dir + '/weak_raw.csv', newline='') as infile:
    reader = csv.reader(infile)
    next(reader)
    for row in reader:
        ps.append(float(row[0]))
        blocking.append(float(row[6]))
        non_blocking.append(float(row[7]))

plt.figure(figsize=(9, 5))
plt.grid()
plt.title("Weak Scaling Study: Iteration")
plt.xlabel("Number of threads, $P =$ scaling factor for $N$")
plt.ylabel("Execution Time (s) with $N = (P \\times 2^{10})$")
plt.autoscale(axis = 'x', tight = True, enable = True)
plt.plot(ps, blocking, 'ro-', label="Blocking")
plt.plot(ps, non_blocking, 'go-', label="Non Blocking")
plt.legend()
plt.savefig(out_dir + '/weak_iterate.svg', dpi=500)

ps = []
insert = []
iterate = []
extract = []
with open(csv_dir + '/strong.csv', newline='') as infile:
    reader = csv.reader(infile)
    next(reader)
    for row in reader:
        ps.append(float(row[0]))
        insert.append(float(row[2]))
        iterate.append(float(row[3]))
        extract.append(float(row[4]))

plt.figure(figsize=(9, 5))
plt.grid()
plt.title("Strong Scaling Study")
plt.xlabel("Number of threads, $P$")
plt.ylabel("Speedup over blocking implementation with $N = 2^{20})$")
plt.autoscale(axis = 'x', tight = True, enable = True)
plt.plot(ps, insert, 'ro-', label="Insertion")
plt.plot(ps, iterate, 'go-', label="Iteration")
plt.plot(ps, extract, 'bo-', label="Extraction")
plt.legend()
plt.savefig(out_dir + '/strong.svg', dpi=500)

ps = []
blocking = []
non_blocking = []
with open(csv_dir + '/strong_raw.csv', newline='') as infile:
    reader = csv.reader(infile)
    next(reader)
    for row in reader:
        ps.append(float(row[0]))
        blocking.append(float(row[6]))
        non_blocking.append(float(row[7]))

plt.figure(figsize=(9, 5))
plt.grid()
plt.title("Strong Scaling Study: Iteration")
plt.xlabel("Number of threads, $P =$ scaling factor for $N$")
plt.ylabel("Execution Time (s) with $N = (P \\times 2^{10})$")
plt.autoscale(axis = 'x', tight = True, enable = True)
plt.plot(ps, blocking, 'ro-', label="Blocking")
plt.plot(ps, non_blocking, 'go-', label="Non Blocking")
plt.legend()
plt.savefig(out_dir + '/strong_iterate.svg', dpi=500)
