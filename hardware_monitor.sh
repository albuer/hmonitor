#!/system/bin/sh

#############################################
# Version Information
#
#   v1.4
#        add ddr clock setup and view ddr load info
#        Display the average value of the load/temp/fps etc.
#   v1.3 add cpu/gpu clocks setup
#        hardware_monitor.sh -c 816 -g 297 # CPU=816MHz, GPU=297MHz
#   v1.2 support vpu clock show
#   v1.1 support rk vr's FPS
#   v1.0 first version
#
#############################################

version=1.4
loop_delay=1
loop_count=0
max_loop_number=0
prev_cpu_use=0
prev_cpu_total=0

cpu_load_total=0
cpu_freq_total=0
cpu_temp_total=0

gpu_load_total=0
gpu_freq_total=0
gpu_temp_total=0

ddr_load_total=0
ddr_freq_total=0

app_fps_total=0
atw_fps_total=0
left_total=0

vpu_freq_total=0
hevc_freq_total=0

skip_first=1
last_log_date=0

new_cpu_clk=0
new_gpu_clk=0
new_ddr_clk=0

title_str="UPTIME(s)\tCPU(F/L/T)\tGPU(F/L/T)\tVPU/HEVC(F)\tDDR(F/L)\tFPS(app/atw/left)"

do_exit()
{
    echo ""
    setprop debug.sf.fps 0
    setprop sys.vr.log 0
#    loop_count=$(($loop_count-1))
#    echo "loop count=$loop_count"
    if [ $loop_count -eq 0 ]; then
        exit 2
    fi

    time_used=$((`date +%s`-$time_begin))

    cpu_load_avg=`echo "$cpu_load_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    cpu_freq_avg=`echo "$cpu_freq_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    cpu_temp_avg=`echo "$cpu_temp_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`

    gpu_load_avg=`echo "$gpu_load_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    gpu_freq_avg=`echo "$gpu_freq_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    gpu_temp_avg=`echo "$gpu_temp_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`

    ddr_load_avg=`echo "$ddr_load_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    ddr_freq_avg=`echo "$ddr_freq_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`

    app_fps_avg=`echo "$app_fps_total $loop_count" | busybox awk '{printf("%.1f", $1/$2)}'`
    atw_fps_avg=`echo "$atw_fps_total $loop_count" | busybox awk '{printf("%.1f", $1/$2)}'`
    left_avg=`echo "$left_total $time_used" | busybox awk '{printf("%.1f", $1/$2)}'`
    
    vpu_freq_avg=`echo "$vpu_freq_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    hevc_freq_avg=`echo "$hevc_freq_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    
    echo "------------------------------------------------------------------------"
    let new_cpu_clk=$new_cpu_clk/1000
    let new_gpu_clk=$new_gpu_clk/1000
    let new_ddr_clk=$new_ddr_clk/1000000

    echo $title_str
    echo "Average($loop_count):\t$cpu_freq_avg/$cpu_load_avg/$cpu_temp_avg\t$gpu_freq_avg/$gpu_load_avg/$gpu_temp_avg\t$vpu_freq_avg/$hevc_freq_avg\t\t$ddr_freq_avg/$ddr_load_avg\t\t$app_fps_avg/$atw_fps_avg/$left_avg"
    echo "Fixed freq: CPU=$new_cpu_clk GPU=$new_gpu_clk DDR=$new_ddr_clk"
    #echo "CPU Load(avg): $cpu_load_avg"
    #echo "GPU Load(avg): $gpu_load_avg"
    #echo "DDR Load(avg): $ddr_load_avg"
    exit 1
}

trap 'do_exit' 1 2 3 6 15

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
while getopts "n:C:D:G:" arg
do
    case $arg in
        C)
            echo "Set CPU clock: $OPTARG MHz"
            let new_cpu_clk=$OPTARG*1000
            if [ "$new_cpu_clk" -eq "0" ]
            then
                echo "interactive" > /sys/bus/cpu/devices/cpu0/cpufreq/scaling_governor
            else
                echo "userspace" > /sys/bus/cpu/devices/cpu0/cpufreq/scaling_governor
                echo $new_cpu_clk > /sys/bus/cpu/devices/cpu0/cpufreq/scaling_setspeed
            fi
            ;;
        D)
            if [ ! -f "/sys/pm_tests/clk_rate" ]
            then
                echo "Unsupport setup DDR clock!"
                echo ""
                exit 1
            fi
            echo "Set DDR clock: $OPTARG MHz"
            let new_ddr_clk=$OPTARG*1000000
            echo "set clk_ddr $new_ddr_clk" > /sys/pm_tests/clk_rate
            ;;
        G)
            if [ ! -f "/sys/devices/ffa30000.gpu/dvfs" ]
            then
                echo "Unsupport setup GPU clock!"
                echo ""
                exit 1
            fi
            echo "Set GPU clock: $OPTARG MHz"
            let new_gpu_clk=$OPTARG*1000
            if [ "$new_gpu_clk" -eq "0" ]
            then
                echo "on" > /sys/devices/ffa30000.gpu/dvfs
            else
                echo "off" > /sys/devices/ffa30000.gpu/dvfs
                echo $new_gpu_clk> /sys/devices/ffa30000.gpu/clock
            fi
            ;;
        n)
            echo "Set Loop number: $OPTARG"
            let max_loop_number=$OPTARG
            ;;
        ?)
            echo "unkonw argument"
            exit 1
            ;;
    esac
done
echo ""

#echo $title_str
setprop debug.sf.fps 1
setprop sys.vr.log 1

time_begin=$((`date +%s`+2))

last_log_date=`date '+%m-%d %H:%M:%S.000'`
#echo "last_log_date=$last_log_date"

while true
do
gpu_load=0
gpu_freq=0
up_time=`uptime | busybox awk '{print substr($3,0,8)}'`
cpu_freq=`cat /sys/bus/cpu/devices/cpu0/cpufreq/cpuinfo_cur_freq`
cpu_freq=$(($cpu_freq/1000))
#echo "CPU Freq: "$cpu_freq" MHz"

eval $(cat /proc/stat | grep "cpu " | busybox awk '
{
    printf("cpu_use=%d; cpu_total=%d;",$2+$3+$4+$6+$7+$8, $2+$3+$4+$5+$6+$7+$8)
}
')

cpu_load=$((($cpu_use-$prev_cpu_use)*100/($cpu_total-$prev_cpu_total)))
#echo "CPU Load: $cpu_load""%"
prev_cpu_use=$cpu_use
prev_cpu_total=$cpu_total

if [ $skip_first -gt 0 ]; then
    #echo "skip first"
    skip_first=0
    sleep 2
    continue
fi

cpu_temp=`cat /sys/devices/ff280000.tsadc/temp1_input`
#echo "CPU Temp: $cpu_temp"

eval $(cat /sys/devices/ffa30000.gpu/dvfs | busybox awk '
{
    if($1=="current_gpu_clk_freq")
        printf("gpu_freq=%d;", $3);
    if($1=="gpu_utilisation")
        printf("gpu_load=%d;", $3); 
    if($5=="gpu_utilisation")
        printf("gpu_load=%d;", $7);
}
')
#echo "GPU Freq: $gpu_freq MHz"
#echo "GPU Load: $gpu_load""%"

gpu_temp=`cat /sys/devices/ff280000.tsadc/temp2_input`
#echo "GPU Temp: $gpu_temp"

eval $(cat /d/clk/clk_summary | grep ddr | busybox awk '{printf("ddr_freq=%d;", $4/1000000);}')
#echo "DDR Freq: $ddr_freq MHz"
ddr_load='0'
if [ -f "/sys/pm_tests/ddrfreq" ]
then
    eval $(cat /sys/pm_tests/ddrfreq | grep "curr_load:" | busybox awk '{printf("ddr_load=%d;", $2);}')
fi

eval $(cat /d/clk/clk_summary | grep " clk_vdpu" | busybox awk '
{
    if($2<=0)
        printf("vdpu_freq=%d;", 0);
    else
        printf("vdpu_freq=%d;", $4/1000000);
}
')

eval $(cat /d/clk/clk_summary | grep " clk_hevc_core" | busybox awk '
{
    if($2<=0)
        printf("hevc_freq=%d;", 0);
    else
        printf("hevc_freq=%d;", $4/1000000);
}
')

#logcat -v time -t "$last_log_date"
app_fps=0
log_date=$last_log_date

#eval $(logcat -v time -t "$last_log_date" | grep -E "SurfaceFlinger.*mFps" | busybox awk -F"=" '{printf("app_fps=%f;", $2);}')
#logcat -v time -t "$last_log_date" | grep -E "SurfaceFlinger.*mFps" | busybox awk '{print $0;}END{printf("Last: %s\n",$0);}'
eval $(logcat -v time -t "$log_date" | grep -E "SurfaceFlinger.*mFps" | busybox awk '
END{
    i=index($0,"mFps = ");
    if(i>0) {
        s=substr($0,i+length("mFps = "));
        printf("app_fps=%s;",s);
        split($2, tm, ":");
        tm[3] += 0.001;
        printf("last_log_date=\"%s %02d:%02d:%.03f\";", $1, tm[1], tm[2], tm[3]);
    }
}
')

#logcat -v time -t "$last_log_date" | grep -E "VRJni.*app.*FPS" | busybox awk '{print $0;}END{printf("NR=%d\n",$NR);}'
eval $(logcat -v time -t "$log_date" | grep -E "VRJni.*app.*FPS" | busybox awk '
END{
    i=index($0,"FPS=");
    if(i>0) {
        s=substr($0,i+length("FPS="));
        printf("app_fps=%s;",s);
    }
}
')

atw_fps=0
left_count=0
eval $(logcat -v time -t "$log_date" | grep -E "VRJni.*atw" | busybox awk '
BEGIN{left_c=0;s=0}
{
    i=index($0,"fps=");
    if(i>0)
        s=substr($0,i+length("fps="));

    if(index($0,"#left")>0)
        ++left_c;
}
END{
    if (length($0)>0) {
        printf("atw_fps=%s;", s);
        printf("left_count=%d;",left_c);
        split($2, tm, ":");
        tm[3] += 0.001;
        printf("last_log_date=\"%s %02d:%02d:%.03f\";", $1, tm[1], tm[2], tm[3]);
    }
}
')

#logcat -v time -t "$aaa" | grep -E "VRJni.*atw" | busybox awk '{print $0;}END{printf("Last: %s\n", $0);}'
#echo "atw_fps=$atw_fps, left_count=$left_count, last_log_date=$last_log_date"
#eval $(logcat -v time -t "$last_log_date" | grep -E "VRJni.*atw.*fps" | busybox awk '{i=index($0,"fps="); if(i>0){s=substr($0,i+length("fps=")); printf("atw_fps=%s;",s);}}')
#left_count=`logcat -t "$last_log_date" | grep -c -E "VRJni.*atw.*#left"`
#last_log_date=`date '+%m-%d %H:%M:%S.000'`

if [ $(($loop_count%20)) -eq 0 ]; then
    echo $title_str
fi

echo "$up_time\t$cpu_freq/$cpu_load/$cpu_temp\t$gpu_freq/$gpu_load/$gpu_temp\t$vdpu_freq/$hevc_freq\t\t$ddr_freq/$ddr_load\t\t$app_fps/$atw_fps/$left_count"

cpu_load_total=$(($cpu_load_total+$cpu_load))
cpu_freq_total=$(($cpu_freq_total+$cpu_freq))
cpu_temp_total=$(($cpu_temp_total+$cpu_temp))

gpu_load_total=$(($gpu_load_total+$gpu_load))
gpu_freq_total=$(($gpu_freq_total+$gpu_freq))
gpu_temp_total=$(($gpu_temp_total+$gpu_temp))

ddr_load_total=$(($ddr_load_total+$ddr_load))
ddr_freq_total=$(($ddr_freq_total+$ddr_freq))

app_fps_total=`echo "$app_fps_total $app_fps" | busybox awk '{print($1+$2)}'`
atw_fps_total=`echo "$atw_fps_total $atw_fps" | busybox awk '{print($1+$2)}'`
left_total=$(($left_total+$left_count))

vpu_freq_total=$(($vpu_freq_total+$vdpu_freq))
hevc_freq_total=$(($hevc_freq_total+$hevc_freq))

loop_count=$(($loop_count+1))

if [ $loop_count -eq $max_loop_number ]; then
    break
fi

sleep $loop_delay
done
do_exit

