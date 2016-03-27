#!/bin/bash

PATH=/usr/local/cross/bin:~/supernova/tools:$PATH
Menu_max=10

function MakeBackup {
	mkdir -p backup
	rar.exe	a -k -ac -m5 -rr20% -se -t -tl -r -ag_DDMMYYYYHHMM -x*.exe ./backup/Supernova ./source ./tools ./binary/Supernova.img ./Supernova.sh
}

function Run_Bochs {
	if [ "$1" -eq "0" ]; then
		#Run normal
		cmd /c start cmd /c "cd d:\!programy\Bochs-2.5.1\ && Bochs.exe -q -f d:\!prog\cygwin\home\Admin\Supernova\bochsrc.bxrc"
	else 
		#Run debuger
		cmd /c start cmd /c "cd d:\!programy\Bochs-2.5.1\ && Bochsdbg.exe -q -f d:\!prog\cygwin\home\Admin\Supernova\bochsrc_db.bxrc"
	fi
}

function Run_VirtualBox {
	if [ "$1" -eq "0" ]; then
		#Run normal
		"/cygdrive/c/Program Files/Oracle/VirtualBox/VirtualBox.exe" -startvm "C:/Users/Admin/VirtualBox VMs/Supernova/Supernova.vbox"
	else 
		#Run debuger
		"/cygdrive/c/Program Files/Oracle/VirtualBox/VirtualBox.exe" --dbg -startvm "C:/Users/Admin/VirtualBox VMs/Supernova/Supernova.vbox"
	fi	
}

function Basic_Menu {
	case "$1" in
		"0")Menu=0	;;
		"1")make -C tools all 
			make -C Source all 
			;;
		"4")cmd /c "start cmd /k" ;;
		"5")Run_Bochs 1 ;;
		"6")make -C Source clean  ;;
		
		"7")Run_Bochs 0 ;;
		"8")Run_VirtualBox 0 ;;
		
		"10")Menu=2	;;
		
		"100")
			echo "0 - Exit"
			echo "1 - Build"
			#echo 3 - Compile and start Bochs
			echo "4 - Start new instance"
			echo "5 - Start debuger"
			echo "6 - Clean"
			echo "7 - Start Bochs"
			echo "8 - Start VirtualBox"
			#echo 8 - Sspace for next emu
			echo "10 - Enter tools menu"	
			Menu_max=10			
			;;
	esac
}

function Tools_Menu {
	case "$1" in
		"0")Menu=1	;;
		"1")make -C tools all  ;;
		"2")
			make -C tools clean 
			make -C tools all 
			;;
		"7")SSM.exe ;;
		"100")
			echo "0 - Return"
			echo "1 - Build tools"
			echo "2 - Rebuild tools"
			
			echo "7 - Run SSM"
			Menu_max=10		
			;;
	esac
}

#cd ~/Supernova
Basic_Menu 6
#MakeBackup
Basic_Menu 1
#cd ~/Supernova

Menu=1
while [ $Menu -ne 0 ]; do
	echo
	echo " =============================================================================="
	case "$Menu" in
		"1") Basic_Menu 100	;;
		"2") Tools_Menu 100	;; 
	esac
	./choice -force -min 0 -max $Menu_max
	case "$Menu" in
		"1") Basic_Menu $?	;;
		"2") Tools_Menu $?	;;
	esac
done

Basic_Menu 6

#BackupArch=$(date +%Y%m%d_%H%M)
#tar -cf -z backup/$BackupArch.tar loop.bat
#Source


