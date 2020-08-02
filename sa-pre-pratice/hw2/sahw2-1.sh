#!/bin/sh
ls -AlRS $PWD | 
sort -nr -k 5 | 
awk NF | 
grep -E "^.{10}\s" | 
awk 'FNR <= 5 {print NR":"$5" "$9} 
/^d/ {dir_sum += 1} END {print "Dir num: "dir_sum} 
/^-/ {file_sum += 1} END {print "File num: "file_sum} 
/^-/ {total_size += $5} END {print "Total: " total_size}'
