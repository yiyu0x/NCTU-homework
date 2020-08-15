#!/bin/sh

list() {
    if [ -z $1 ];then #Not other argv, only has -l 
        zfs list -t snapshot -o name,creation -s creation | \
            awk '{if(NR>1) print $1}' | \
            awk -F "@" 'BEGIN {printf "%-3s %-20s %-20s\n", "ID", "DATASET", "TIME"}
                {printf "%-3s %-20s %-20s\n", NR, $1, $2}'
    else
        echo $1 | grep -q "^[0-9]*$"
        if [ $? = 0 ];then #-l and ID is set
                zfs list -t snapshot -o name,creation -s creation | \
                    awk -v id=$1 '{if(NR-1==id) print $1}' | \
                    awk -v id=$1 -F "@" 'BEGIN {printf "%-3s %-20s %-20s\n", "ID", "DATASET", "TIME"}
                        {printf "%-3s %-20s %-20s\n", id, $1, $2}'
        else #-l and dataset is set
            dataset=$1
            if [ -z $2 ];then #Not set dataset ID
                zfs list -t snapshot -o name,creation -s creation | \
                    grep $dataset |\
                    awk '{print $1}' | \
                    awk -F "@" 'BEGIN {printf "%-3s %-20s %-20s\n", "ID", "DATASET", "TIME"}
                        {printf "%-3s %-20s %-20s\n", NR, $1, $2}'
            else
                id=$2
                zfs list -t snapshot -o name,creation -s creation | \
                    grep $dataset |\
                    awk -v id=$id '{if(NR==id) print $1}' | \
                    awk -v id=$id -F "@" 'BEGIN {printf "%-3s %-20s %-20s\n", "ID", "DATASET", "TIME"}
                        {printf "%-3s %-20s %-20s\n", id, $1, $2}'
            fi
        fi
    fi
}

delete() {
    if [ -z $1 ];then #Not other argv, only has -d
        list | awk '{if(NR>1) print $2"@"$3}' | xargs -L1 echo Destroy
        list | awk '{if(NR>1) print $2"@"$3}' | xargs -L1 zfs destroy
    else
        echo $1 | grep -q "^[0-9]*$"
	    if [ $? = 0 ];then #Specific ID
	        target_name=$(list | awk NR==$1+1'{print}' | awk '{print $2"@"$3}')
	        zfs destroy $target_name
	        echo "Destroy $target_name"
	    else #Specific dataset
	        if [ -z $2 ];then #Only specific dataset
	            list | grep $1 | awk '{print $2"@"$3}' | xargs -L1 echo Destroy
	            list | grep $1 | awk '{print $2"@"$3}' | xargs -L1 zfs destroy
	        else #Specific dataset and ID
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
    tmpfile=$(mktemp /tmp/backup.gz.XXX)
    zfs send $target_name | gzip > $tmpfile
    save_target_name=$(echo $target_name | tr "/" "-")
    openssl aes-256-cbc -a -salt -pbkdf2 -in $tmpfile -out "/tmp/$save_target_name.gz.enc"
    echo "Export $target_name to /tmp/$save_target_name.gz.enc"
    rm $tmpfile
}

import() {
    filename=$1
    dataset=$2
    tmpfile=$(mktemp -u /tmp/backup.XXX)
    tmpfilegz="$tmpfile.gz"
    openssl aes-256-cbc -d -a -pbkdf2 -in $filename -out $tmpfilegz
    gunzip $tmpfilegz
    zfs receive -F $dataset < $tmpfile
    rm $tmpfile
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
    -l) list $2 $3;;
    -d) delete $2 $3;;
    -e) zexport $2 $3;;
    -i) import $2 $3;;
    *) create $1 $2;;
esac
