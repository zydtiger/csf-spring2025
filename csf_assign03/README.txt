# Assignment 3

## Students
- Tiger Ding
- Zihao Zhao

Best cache report

The following experiments are running over the gcc.trace. 
Writing policy:
nwa = no-write-allocate
wa = write-allocate
wb = write-back
wt = write-through

We first compared two cache configurations: one with 256 sets, 4 blocks per set, and a 16-byte block size, and another with 1024 sets, 1 block per set, and the same block size. 
Both configurations have the same total cache size of 16 KB. 
To ensure a fair comparison, the write and eviction policies were kept identical. 
This comparison primarily focuses on evaluating the performance differences between 4-way set-associative and direct-mapped caches.
Group 1:
256_4_16_wa_wb_fifo
Total loads:    318197
Total stores:   197486
Load hits:      314171
Load misses:    4026
Store hits:     188047
Store misses:   9439
Total cycles:   10033330


1024_1_16_wa_wb_fifo
Total loads:    318197
Total stores:   197486
Load hits:      312238
Load misses:    5959
Store hits:     187502
Store misses:   9984
Total cycles:   26828985

Group2:
256_4_16_wa_wb_lru
Total loads:    318197
Total stores:   197486
Load hits:      314798
Load misses:    3399
Store hits:     188250
Store misses:   9236
Total cycles:   9532733

1024_1_16_wa_wb_lru
Total loads:    318197
Total stores:   197486
Load hits:      312238
Load misses:    5959
Store hits:     187502
Store misses:   9984
Total cycles:   26828985

Group 3:
256_4_16_nwa_wt_fifo
Total loads:    318197
Total stores:   197486
Load hits:      311017
Load misses:    7180
Store hits:     163705
Store misses:   33781
Total cycles:   23299988

1024_1_16_nwa_wt_fifo
Total loads:    318197
Total stores:   197486
Load hits:      309108
Load misses:    9089
Store hits:     162581
Store misses:   34905
Total cycles:   24062464

Conclusion:
There is a significant difference in total cycles between direct-mapped and 4-way set-associative caches in both Group 1 and Group 2. 
In Group 1, the 4-way associative cache results in approximately 10 million total cycles, while the direct-mapped cache requires around 26 million cycles. 
Additionally, across all three group comparisons, the 4-way associative cache consistently achieves higher load hit and store hit rates compared to the direct-mapped cache. 
These results indicate that the 4-way associative cache provides better overall performance.


We will use the same cache configuration in the following experiments. 
This time, we focus on comparing different write policies. 
In each group, the mapping and eviction policies are kept consistent to ensure a fair comparison.



Group 1:
256_4_16_wa_wb_fifo
Total loads:    318197
Total stores:   197486
Load hits:      314171
Load misses:    4026
Store hits:     188047
Store misses:   9439
Total cycles:   10033330

256_4_16_nwa_wt_fifo
Total loads:    318197
Total stores:   197486
Load hits:      311017
Load misses:    7180
Store hits:     163705
Store misses:   33781
Total cycles:   23299988

256_4_16_wa_wt_fifo
Total loads:    318197
Total stores:   197486
Load hits:      314171
Load misses:    4026
Store hits:     188047
Store misses:   9439
Total cycles:   25838330

Group 2:
256_4_16_wa_wb_lru
Total loads:    318197
Total stores:   197486
Load hits:      314798
Load misses:    3399
Store hits:     188250
Store misses:   9236
Total cycles:   9532733

256_4_16_nwa_wt_lru
Total loads:    318197
Total stores:   197486
Load hits:      311613
Load misses:    6584
Store hits:     164819
Store misses:   32667
Total cycles:   23062702

256_4_16_nwa_wt_lru
Total loads:    318197
Total stores:   197486
Load hits:      314798
Load misses:    3399
Store hits:     188250
Store misses:   9236
Total cycles:   25506533

Group 3:
1024_1_16_wa_wb_fifo
Total loads:    318197
Total stores:   197486
Load hits:      312238
Load misses:    5959
Store hits:     187502
Store misses:   9984
Total cycles:   26828985

1024_1_16_nwa_wt_fifo
Total loads:    318197
Total stores:   197486
Load hits:      309108
Load misses:    9089
Store hits:     162581
Store misses:   34905
Total cycles:   24062464

1024_1_16_wa_wt_fifo
Total loads:    318197
Total stores:   197486
Load hits:      312238
Load misses:    5959
Store hits:     187502
Store misses:   9984
Total cycles:   26828985

Among the three groups compared, the no-write-allocate write-through policy performs the best under the direct mapping. 
Its total cycle count is at least 2 million lower than that of the other write policies. 
Although its hit rate is slightly lower—by approximately 10,000 hits—the difference is relatively small compared to the overall number of hits, making it a worthwhile trade-off for improved performance.
However, under set-associative mapping, the write-allocate write-back policy yields the best performance, with a total cycle count that is nearly 10 million lower than the other policies.



Finally, we will compare the eviction policy (lru vs fifo)
We will use the same cache configuration in the following experiments. 
Also, the writing policy in each group will be the same. 

Group 1:
256_4_16_wa_wb_fifo
Total loads:    318197
Total stores:   197486
Load hits:      314171
Load misses:    4026
Store hits:     188047
Store misses:   9439
Total cycles:   10033330

256_4_16_wa_wb_lru
Total loads:    318197
Total stores:   197486
Load hits:      314798
Load misses:    3399
Store hits:     188250
Store misses:   9236
Total cycles:   9532733

Group 2:
1024_1_16_wa_wb_fifo
Total loads:    318197
Total stores:   197486
Load hits:      312238
Load misses:    5959
Store hits:     187502
Store misses:   9984
Total cycles:   26828985

1024_1_16_wa_wb_lru
Total loads:    318197
Total stores:   197486
Load hits:      312238
Load misses:    5959
Store hits:     187502
Store misses:   9984
Total cycles:   26828985

Conclusion:
In Group 2, the statistics for both FIFO and LRU replacement policies are identical. 
In Group 1, however, LRU achieves lower total cycles while maintaining similar hit rates compared to FIFO. 
Therefore, LRU is the preferred replacement policy in this case.

Final Remark:
From the comparisons above, we conclude that associative mapping generally provides better performance than direct mapping. 
Based on this configuration, the write-allocate write-back policy performs best. 
Additionally, LRU tends to outperform FIFO in most scenarios.

