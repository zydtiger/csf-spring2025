CONTRIBUTIONS

- Tiger Ding: Wrote the code for functions
- Zihao Zhao: Ran the experiments and wrote the report

REPORT
Test run with threshold 2097152

real    0m0.383s
user    0m0.372s
sys     0m0.010s
Test run with threshold 1048576

real    0m0.241s
user    0m0.396s
sys     0m0.030s
Test run with threshold 524288

real    0m0.156s
user    0m0.421s
sys     0m0.038s
Test run with threshold 262144

real    0m0.132s
user    0m0.448s
sys     0m0.047s
Test run with threshold 131072

real    0m0.124s
user    0m0.448s
sys     0m0.061s
Test run with threshold 65536

real    0m0.114s
user    0m0.461s
sys     0m0.084s
Test run with threshold 32768

real    0m0.134s
user    0m0.482s
sys     0m0.137s
Test run with threshold 16384

real    0m0.143s
user    0m0.529s
sys     0m0.162s


With parallelization, we can sort data more efficiently by creating multiple child processes that handle smaller parts of the array concurrently. The number of parallel processes created depends on the threshold value: a smaller threshold means that even smaller subarrays will be sorted in parallel, leading to the creation of more processes.

In our experiment, the first six tests showed a clear decrease in execution time as the threshold decreased. This is because more processes were created, allowing better utilization of available CPU cores and improving overall efficiency.

However, in the last three tests, the execution time started to increase instead of decrease. This is due to hardware limitations—specifically, the limited number of CPU cores. Once the number of processes exceeds the number of available cores, the system can no longer run all processes simultaneously. Instead, it starts time-sharing the CPU using context switching (i.e., switching between processes at regular intervals).

This context switching introduces overhead and reduces the benefits of parallelism. As a result, although more processes are created with smaller thresholds, they do not run in true parallel. Instead, they compete for CPU time, and the additional switching overhead actually increases the total execution time.

In summary, while reducing the threshold can initially speed up sorting by leveraging parallelism, setting it too low causes an overhead from excessive process creation and CPU time-sharing, which negates the benefits and leads to longer execution times.

