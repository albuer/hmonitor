#!/system/bin/sh

#############################################
# Version Information
#
#   v1.3 add cpu/gpu clocks setup
#        hardware_monitor.sh -c 816 -g 297 # CPU=816MHz, GPU=297MHz
#   v1.2 support vpu clock show
#   v1.1 support rk vr's FPS
#   v1.0 first version
#
#############################################

version=1.3
loop_delay=1
old_cpu_use=0
old_cpu_total=0
title_intervel=0
title_str="UP TIME(s)\tCPU(F/L/T)\tGPU(F/L/T)\tVPU/HEVC(F)\tDDR(F)\tFPS"

echo ""
echo "Hardware Monitor for RK3288 VR, Version: "$version
echo "\tF - Freq(MHz)"
echo "\tL - Load(%)"
echo "\tT - Temperature(C)"
echo ""
echo "[Model]: "`getprop ro.product.model`
echo "[Firmware]: "`getprop ro.build.description`
echo "[Kernel]: "`cat /proc/version`
echo ""

#dmesg -t | grep '|-' | while read curr
#do
#    echo $curr
#done

new_cpu_clk=0
new_gpu_clk=0
while getopts "c:g:" arg
do
    case $arg in
        c)
            echo "Set CPU clock: $OPTARG MHz"
            let new_cpu_clk=$OPTARG*1000
            if [ "$new_cpu_clk" -eq "0" ]
            then
                echo "interactive" > /sys/bus/cpu/devices/cpu0/cpufreq/scaling_governor
            else
                echo "userspace" > /sys/bus/cpu/devices/cpu0/cpufreq/scaling_governor
                echo $new_cpu_clk > /sys/bus/cpu/devices/cpu0/cpufreq/scaling_setspeed
            fi
            echo "";;
        g)
            echo "Set GPU clock: $OPTARG MHz"
            let new_gpu_clk=$OPTARG*1000
            if [ "$new_gpu_clk" -eq "0" ]
            then
                echo "on" > /sys/devices/ffa30000.gpu/dvfs
            else
                echo "off" > /sys/devices/ffa30000.gpu/dvfs
                echo $new_gpu_clk> /sys/devices/ffa30000.gpu/clock
            fi
            echo "";;
        ?)
            echo "unkonw argument"
            exit 1
            ;;
    esac
done

echo $title_str
setprop debug.sf.fps 1
setprop sys.vr.log 1

while true
do
gpu_load=0
gpu_freq=0
up_time=`uptime | busybox awk '{print substr($3,0,8)}'`
cpu_freq=`cat /sys/bus/cpu/devices/cpu0/cpufreq/cpuinfo_cur_freq`
cpu_freq=$(($cpu_freq/1000))
#echo "CPU Freq: "$cpu_freq" MHz"

eval $(cat /proc/stat | grep "cpu " | busybox awk '{printf("cpu_use=%d; cpu_total=%d;",$2+$3+$4+$6+$7+$8, $2+$3+$4+$5+$6+$7+$8)}')
cpu_load=$((($cpu_use-$old_cpu_use)*100/($cpu_total-$old_cpu_total)))
#echo "CPU Load: $cpu_load""%"
old_cpu_use=$cpu_use
old_cpu_total=$cpu_total

cpu_temp=`cat /sys/devices/ff280000.tsadc/temp1_input`
#echo "CPU Temp: $cpu_temp"

eval $(cat /sys/devices/ffa30000.gpu/dvfs | busybox awk '{if($1=="current_gpu_clk_freq") printf("gpu_freq=%d;", $3); if($1=="gpu_utilisation") printf("gpu_load=%d;", $3); if($5=="gpu_utilisation") printf("gpu_load=%d;", $7);}')
#echo "GPU Freq: $gpu_freq MHz"
#echo "GPU Load: $gpu_load""%"

gpu_temp=`cat /sys/devices/ff280000.tsadc/temp2_input`
#echo "GPU Temp: $gpu_temp"

eval $(cat /d/clk/clk_summary | grep ddr | busybox awk '{printf("ddr_freq=%d;", $4/1000000);}')
#echo "DDR Freq: $ddr_freq MHz"

eval $(cat /d/clk/clk_summary | grep " clk_vdpu" | busybox awk '{if($2<=0) printf("vdpu_freq=%d;", 0); else printf("vdpu_freq=%d;", $4/1000000);}')
eval $(cat /d/clk/clk_summary | grep " clk_hevc_core" | busybox awk '{if($2<=0) printf("hevc_freq=%d;", 0); else printf("hevc_freq=%d;", $4/1000000);}')

fps=0
eval $(logcat -d -s SurfaceFlinger | grep mFps | busybox awk -F"=" '{printf("fps=%f;", $2);}')
eval $(logcat -d -s VRJni | grep fps | busybox awk '{i=index($0,"fps="); if(i>0){s=substr($0,i+length("fps=")); printf("fps=%s;",s);}}')
#echo "fps: $fps"

if [ $title_intervel -gt 20 ]; then
echo $title_str
title_intervel=0
fi
echo "$up_time\t$cpu_freq/$cpu_load/$cpu_temp\t$gpu_freq/$gpu_load/$gpu_temp\t$vdpu_freq/$hevc_freq\t\t$ddr_freq\t$fps"
title_intervel=$(($title_intervel+1))
logcat -c
sleep $loop_delay
done
