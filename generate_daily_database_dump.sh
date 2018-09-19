#!/bin/bash
day=$(date --date=${dateinfile#?_} "+%A")
mysqldump -h malloc-db -P 3309 --lock-tables=false mm6_fits | gzip -9 -c > /nfs/team112_data02/magnus/krempel/dumps/$day.sql.gz
