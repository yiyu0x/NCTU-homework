#!/bin/sh

list() {
    zfs list -t snapshot -o name,creation -s creation | \
        awk '{if(NR>1) print $1}' | \
        awk -F "@" 'BEGIN {printf "%-3s %-20s %-20s\n", "ID", "DATASET", "TIME"} 
            {printf "%-3s %-20s %-20s\n", NR ,$1, $2}'
}

delete() {
    echo $1 | grep -q "^[0-9]*$"
    if [ -z $1 ];then #Not other argv, only has -d
        list | awk '{print $2"@"$3}' | xargs -L1 echo Destroy
        list | awk '{print $2"@"$3}' | xargs -L1 zfs destroy
    else
	    if [ $? = 0 ];then
	        target_name=$(list | awk NR==$1+1'{print}' | awk '{print $2"@"$3}')
	        zfs destroy $target_name
	        echo "Destroy $target_name"
	    else #Specific dataset
	        if [ -z $2 ];then #Only specific dataset
	            list | grep $1 | awk '{print $2"@"$3}' | xargs -L1 echo Destroy
	            list | grep $1 | awk '{print $2"@"$3}' | xargs -L1 zfs destroy
	        else
                list | grep $1 | awk '{print $2"@"$3}' | awk "{if(NR==$2) print}" | xargs -L1 echo Destroy
	            list | grep $1 | awk '{print $2"@"$3}' | awk "{if(NR==$2) print}" | xargs -L1 zfs destroy
	        fi
	    fi
    fi
}

zexport() {
    dataset=$1
    if [ -z $2 ];then
        id=1
    else
        id=$2
    fi
    target_name=$(list | grep $dataset | awk '{print $2"@"$3}' | awk "{if(NR==$id) print}")
    zfs send $target_name | gzip > /tmp/backupfile.gz
    save_target_name=$(echo $target_name | tr "/" "-")
    openssl aes-256-cbc -a -salt -in /tmp/backupfile.gz -out "/tmp/$save_target_name.gz.enc"
    echo "Export $target_name to /tmp/$save_target_name.gz.enc"
}

import() {
    filename=$1
    dataset=$2
    openssl aes-256-cbc -d -a -in $filename -out /tmp/backupfile.gz
    gunzip /tmp/backupfile.gz
    zfs receive -F $dataset < /tmp/backupfile
}

create() {
    dataset=$1
    if [ -z $2 ];then
        rotation_count=20
    else
        rotation_count=$2
    fi
    snapshot_name=$(date +"%Y-%m-%d-%H:%M:%S")
    zfs snapshot -r $dataset@$snapshot_name
    echo "Snap $dataset@$snapshot_name"
    lines=$(list | wc -l)
    lines=$((lines-1))
    list | grep $1 | awk -v count=$rotation_count -v lines=$lines '{if(NR<=lines-count) print $2"@"$3}' | xargs -L1 echo Destroy
    list | grep $1 | awk -v count=$rotation_count -v lines=$lines '{if(NR<=lines-count) print $2"@"$3}' | xargs -L1 zfs destroy
}

argv=$1
case $argv in
    -l) list;;
    -d) delete $2 $3;;
    -e) zexport $2 $3;;
    -i) import $2 $3;;
    *) create $1 $2;;
esac
