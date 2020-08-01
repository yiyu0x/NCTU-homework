#!/bin/bash
GLOBIGNORE=*
cpu_info(){
    cpu_model=`sysctl -a | grep hw.model: | cut -d : -f 2`
    cpu_machine=`sysctl -a | grep hw.machine: | cut -d : -f 2`
    cpu_core=`sysctl -a | grep hw.ncpu: | cut -d : -f 2`
    dialog --msgbox "CPU Info\n\n
CPU Model: $cpu_model\n\n
CPU Machine: $cpu_machine\n\n
CPU Core: $cpu_core" 50 70
}

memory_info(){
    unit_converter(){
		local counter=0
		local mem=$1
		local mem_float=$1
		while [ $((mem/1024)) -gt 0 ]
		do
			counter=$((counter+1))
			mem=$((mem/1024))
			mem_float=$(bc <<< "scale=2; $mem_float/1024")
		done
		case $counter in
			1) ret_val="$mem_float KB"
				;;
			2) ret_val="$mem_float MB"
				;;
			3) ret_val="$mem_float GB"
				;;
		esac
	}
    total_mem=$(sysctl hw | grep -E 'hw.realmem' | awk '{print $2}')
    used_mem=$(sysctl hw | grep -E 'hw.user' | awk '{print $2}')
    free_mem=$((total_mem-$used_mem))
    
    unit_converter $total_mem
    total_mem_f=$ret_val
    unit_converter $used_mem
    used_mem_f=$ret_val
    unit_converter $free_mem
    free_mem_f=$ret_val
    
    percent=$(echo $(bc <<< "scale=2; ($used_mem/$total_mem)*100") / 1 | bc)
    while true; do
        dialog --mixedgauge "Memory Info and Usage\n\n
Total: $total_mem_f\n
Used: $used_mem_f\n
Free: $free_mem_f" 50 70 $percent 
	    read -t 3 key
	    VALID=$?
	    if [ $VALID = 0 ]; then 
		    break; 
	    fi
    done
}

network_info(){
    raw_string=$(ifconfig | awk -F':' '/^[a-zA-Z]/ {print $1 " x"}')
    while true; do
        selected=$(dialog --menu "Network Interfaces" 50 70 70 \
                    $(ifconfig | awk -F':' '/^[a-zA-Z]/ {print $1 " *"}') \
                        --output-fd 1)
        if [ "$?" = 0 ]
        then
            ipv4_addr=$(ifconfig em0 | awk '/\tinet.*netmask.*broadcast/ {print $2}')
            netmask=$(ifconfig em0 | awk '/\tinet.*netmask.*broadcast/ {print $4}')
            mac_addr=$(ifconfig em0 | awk '/\tether/ {print $2}')
            dialog --msgbox "Interface Name: $selected\n\n
IPV4___: $ipv4_addr\n
Netmask: $netmask\n
Mac____: $mac_addr" 50 70
        else
            break
        fi    
    done
}


file_browser(){
    file_info(){
        file_name=$1
        fname="<File Name>: "$(basename $file_name)"\n\n"
        finfo="<File Info>: "$(file $file_name | cut -d ':' -f 2)"\n\n"
        fsize="<File Size>: "$(ls -lh $file_name | cut -d ' ' -f 9)
        info_str=$fname$finfo$fsize
        file $file_name | grep -q "text" 
        is_text=$?
        if [ $is_text = "0" ] # is a text file
        then
            while true; do
                dialog --yes-label 'OK' --no-label 'Edit' --yesno "$info_str" 50 70
                edit=$?
                if [ $edit = "1" ]
                then
                    $EDITOR $file_name
                else
                    break
                fi
            done
        else # not a text file
            dialog --yes-label 'OK' --yesno "$info_str" 50 70
        fi
    }
    dir=$1
    files=$(find $dir -maxdepth 1 | xargs basename | xargs file -i | cut -d \; -f 1 | tr -d : | awk 'NR==2 {print ".. inode/directory" } {print}')
    while true; do
        selected_file=$(dialog --menu "File Browser: $PWD" 50 70 70 $files --output-fd 1)
        if [ $? != "0" ]
        then
            break 
        elif [ $selected_file = ".." ]
        then
            cd ..
            file_browser .
            break
        elif [ -d $selected_file ] 
        then
            cd $selected_file
            file_browser .
            break
        else
            file_info $selected_file
        fi
    done
}

curr_dir=$PWD
while true; do
    cd $curr_dir
    selected=$(dialog --title "SYS INFO" --menu "" 50 70 70 \
        1 "CPU INFO" \
        2 "MEMORY INFO" \
        3 "NETWORK INFO" \
        4 "FILE BROWSER" \
        --output-fd 1)
    if [ "$?" = 0 ]
    then 
        case $selected in
            1) cpu_info;;
            2) memory_info;;
            3) network_info;;
            4) file_browser .;;
        esac
    else
        exit 0
        clear
    fi
done
