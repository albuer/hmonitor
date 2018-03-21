#!/system/bin/sh

#############################################
#
# Version Information
#
#   v1.8 Support RK3326@Linux4.4
#   v1.7 Compatible RK3399@Linux4.4, and use function rewrite code
#   v1.6 disable VR mode by default, use "-V" to enable it.
#   v1.5 output sensor's min/avg/max latency
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

version=1.8

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

sensor_min_total=0
sensor_avg_total=0
sensor_max_total=0

vpu_freq_total=0
hevc_freq_total=0

skip_first=1
last_log_date=0

new_cpu_clk=0
new_gpu_clk=0
new_ddr_clk=0

vr_mode=0

get_cpu_info()
{
    cpu_freq=0
    cpu_load=0
    cpu_temp=0

    if [ -f "/sys/bus/cpu/devices/cpu0/cpufreq/cpuinfo_cur_freq" ]; then
        cpu_freq=`cat /sys/bus/cpu/devices/cpu0/cpufreq/cpuinfo_cur_freq`
        cpu_freq=$(($cpu_freq/1000))
    fi

    eval $(cat /proc/stat | grep "cpu " | busybox awk '
    {
        printf("cpu_use=%d; cpu_total=%d;",$2+$3+$4+$6+$7+$8, $2+$3+$4+$5+$6+$7+$8)
    }
    ')
    cpu_load=$((($cpu_use-$prev_cpu_use)*100/($cpu_total-$prev_cpu_total)))

    if [ -f "/sys/devices/ff280000.tsadc/temp1_input" ]; then # rk3288
        cpu_temp=`cat /sys/devices/ff280000.tsadc/temp1_input`
    elif [ -f "/sys/devices/11150000.tsadc/temp0_input" ]; then # rk3228
        cpu_temp=`cat /sys/devices/11150000.tsadc/temp0_input`
    elif [ -f "/sys/class/thermal/thermal_zone0/temp" ]; then # rk3399 @ linux4.4
        cpu_temp=`cat /sys/class/thermal/thermal_zone0/temp`
        cpu_temp=$(($cpu_temp/1000))
    fi
}

get_gpu_info()
{
    gpu_freq=0
    gpu_load=0
    gpu_temp=0

    if [ -f "/sys/devices/$gpu_name/dvfs" ]; then
        eval $(cat /sys/devices/$gpu_name/dvfs | busybox awk '
        {
        if($1=="current_gpu_clk_freq")
            printf("gpu_freq=%d;", $3);
        if($1=="gpu_utilisation")
            printf("gpu_load=%d;", $3); 
        if($5=="gpu_utilisation")
            printf("gpu_load=%d;", $7);
        }
        ')
    elif [ -f "/sys/devices/platform/$gpu_name/devfreq/$gpu_name/load" ]; then
        eval $(cat /sys/devices/platform/$gpu_name/devfreq/$gpu_name/load | busybox awk -F@ '
        {
            printf("gpu_load=%d;", $1);
            printf("gpu_freq=%d;", ($2+0)/1000000);
        }')
    fi

    if [ -f "/sys/devices/ff280000.tsadc/temp2_input" ]; then
        gpu_temp=`cat /sys/devices/ff280000.tsadc/temp2_input`
    elif [ -f "/sys/class/thermal/thermal_zone1/temp" ]; then # rk3399 @ linux4.4
        gpu_temp=`cat /sys/class/thermal/thermal_zone1/temp`
        gpu_temp=$(($gpu_temp/1000))
    fi
}

get_ddr_info()
{
    ddr_freq=0
    eval $(cat /d/clk/clk_summary | grep sclk_ddrc | busybox awk '{printf("ddr_freq=%d;", $4/1000000);}') # rk3399 @ linux4.4
    if [ $ddr_freq == 0 ]; then
        eval $(cat /d/clk/clk_summary | grep ddr | busybox awk '{printf("ddr_freq=%d;", $4/1000000);}')
    fi

    ddr_load='0'
    if [ -f "/sys/pm_tests/ddrfreq" ]; then
        eval $(cat /sys/pm_tests/ddrfreq | grep "curr_load:" | busybox awk '{printf("ddr_load=%d;", $2);}')
    fi
}

get_vpu_info()
{
    vdpu_freq=0
    eval $(cat /d/clk/clk_summary | grep " clk_vdpu" | busybox awk '
    {
        if($2<=0)
            printf("vdpu_freq=%d;", 0);
        else
            printf("vdpu_freq=%d;", $4/1000000);
    }
    ')
    if [ $vdpu_freq == 0 ]; then
    eval $(cat /d/clk/clk_summary | grep " aclk_vpu " | busybox awk '
    {
        if($2<=0)
            printf("vdpu_freq=%d;", 0);
        else
            printf("vdpu_freq=%d;", $4/1000000);
    }
    ')
    fi
    if [ $vdpu_freq == 0 ]; then
    eval $(cat /d/clk/clk_summary | grep " aclk_vcodec" | busybox awk '# 3399@Linux4.4
    {
        if($2<=0)
            printf("vdpu_freq=%d;", 0);
        else
            printf("vdpu_freq=%d;", $4/1000000);
    }
    ')
    fi

    hevc_freq=0
    eval $(cat /d/clk/clk_summary | grep " clk_hevc_core" | busybox awk '
    {
        if($2<=0)
            printf("hevc_freq=%d;", 0);
        else
            printf("hevc_freq=%d;", $4/1000000);
    }
    ')
}

get_disp_fps()
{
    app_fps=0
    log_date=$last_log_date

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
}

get_vr_fps()
{
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
    eval $(logcat -v time -t "$log_date" | grep -E "VRJni.*atw" | grep -v "direct=" | busybox awk '
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

    eval $(logcat -v time -t "$log_date" | grep -E "VRJni.*direct.*fps" | busybox awk '
    END{
        i=index($0,"fps=");
        if(i>0) {
            s=substr($0,i+length("fps="));
            printf("app_fps=%s;", s);
            printf("atw_fps=%d;", 0);
            printf("left_count=%d;", 0);
        }
    }
    ')
}

get_sensor_info()
{
    sensor_min=0
    sensor_avg=0
    sensor_max=0
    eval $(logcat -v time -t "$sensor_log_date" | grep -E "SensorManager.*Client Time" | busybox awk '
    END{
        i=index($0,"] ");
        if(i>0) {
            s=substr($0,i+length("] "));
            #printf("sensor_latency=%s;",s);
            split(s, sl, ",");
            #printf("sensor_latency=%d/%d/%d;", sl[1]/1000,sl[2]/1000,sl[3]/1000);
            printf("sensor_min=%d;", sl[1]/1000);
            printf("sensor_avg=%d;", sl[2]/1000);
            printf("sensor_max=%d;", sl[3]/1000);
            split($2, tm, ":");
            tm[3] += 0.001;
            printf("sensor_log_date=\"%s %02d:%02d:%.03f\";", $1, tm[1], tm[2], tm[3]);
        }
    }
    ')
}

do_exit()
{
    echo ""
    setprop debug.sf.fps 0
    setprop debug.hwc.logfps 0
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

    vpu_freq_avg=`echo "$vpu_freq_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    hevc_freq_avg=`echo "$hevc_freq_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    
    app_fps_avg=`echo "$app_fps_total $loop_count" | busybox awk '{printf("%.1f", $1/$2)}'`

    if [ $vr_mode == 1 ]; then
        setprop sys.vr.log 0
        #setprop sensor.debug.time 0

        atw_fps_avg=`echo "$atw_fps_total $loop_count" | busybox awk '{printf("%.1f", $1/$2)}'`
        left_avg=`echo "$left_total $time_used" | busybox awk '{printf("%.1f", $1/$2)}'`
    
        sensor_min_avg=`echo "$sensor_min_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
        sensor_avg_avg=`echo "$sensor_avg_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
        sensor_max_avg=`echo "$sensor_max_total $loop_count" | busybox awk '{printf("%d", $1/$2+0.5)}'`
    fi
    #echo "------------------------------------------------------------------------"
    let new_cpu_clk=$new_cpu_clk/1000
    let new_gpu_clk=$new_gpu_clk/1000
    let new_ddr_clk=$new_ddr_clk/1000000

    echo $title_str
    if [ $vr_mode == 1 ]; then
        busybox printf "Average(%d):\t%03d/%02d/%02d\t%03d/%02d/%02d\t%03d/%03d\t\t%03d/%02d\t\t%03d/%03d/%03d\t%.1f/%.1f/%.1f\n" $loop_count $cpu_freq_avg $cpu_load_avg $cpu_temp_avg $gpu_freq_avg $gpu_load_avg $gpu_temp_avg $vpu_freq_avg $hevc_freq_avg $ddr_freq_avg $ddr_load_avg $sensor_min_avg $sensor_avg_avg $sensor_max_avg $app_fps_avg $atw_fps_avg $left_avg
    else
        busybox printf "Average(%d):\t%03d/%02d/%02d\t%03d/%02d/%02d\t%03d/%03d\t\t%03d/%02d\t\t%.1f\n" $loop_count $cpu_freq_avg $cpu_load_avg $cpu_temp_avg $gpu_freq_avg $gpu_load_avg $gpu_temp_avg $vpu_freq_avg $hevc_freq_avg $ddr_freq_avg $ddr_load_avg $app_fps_avg
    fi

    echo "Fixed freq: CPU=$new_cpu_clk GPU=$new_gpu_clk DDR=$new_ddr_clk"

    exit 0
}

shwo_usage()
{
echo ""
echo "Usage: $0 [OPTION]..."
echo "  -C clock\t\tSetup CPU frequency at MHz"
echo "  -G clock\t\tSetup GPU frequency at MHz"
echo "  -D clock\t\tSetup DDR frequency at MHz"
echo "  -V \t\t\tfor VR product"
echo "  -n num\t\tLoop count"
}

trap 'do_exit' 1 2 3 6 15

echo ""
echo "Hardware Monitor for Rockchip platform, Version: "$version
echo "\tF - Freq(MHz)"
echo "\tL - Load(%)"
echo "\tT - Temperature(C)"
echo ""
echo "[Model]: "`getprop ro.product.model`
echo "[Firmware]: "`getprop ro.build.description`
echo "[Kernel]: "`cat /proc/version`

gpu_name=''
eval $(cat /proc/iomem | grep gpu | busybox awk '{printf("gpu_name=%s;", $3);}')

while getopts "n:C:D:G:V" arg
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
            if [ ! -f "/sys/devices/$gpu_name/dvfs" ]
            then
                echo "Unsupport setup GPU clock!"
                echo ""
                exit 1
            fi
            echo "Set GPU clock: $OPTARG MHz"
            let new_gpu_clk=$OPTARG*1000
            if [ "$new_gpu_clk" -eq "0" ]
            then
                echo "on" > /sys/devices/$gpu_name/dvfs
            else
                echo "off" > /sys/devices/$gpu_name/dvfs
                echo $new_gpu_clk> /sys/devices/$gpu_name/clock
            fi
            ;;
        n)
            echo "Set Loop number: $OPTARG"
            let max_loop_number=$OPTARG
            ;;
        V)
            echo "Monitor for VR"
            vr_mode=1
            ;;
        ?)
            shwo_usage
            exit 1
            ;;
    esac
done
echo ""

setprop debug.sf.fps 1
setprop debug.hwc.logfps 1
time_begin=$((`date +%s`+2))
last_log_date=`date '+%m-%d %H:%M:%S.000'`

if [ $vr_mode == 1 ]; then
    title_str="\nUPTIME(s)\tCPU(F/L/T)\tGPU(F/L/T)\tVPU/HEVC(F)\tDDR(F/L)\tSENSOR(us)\tFPS(app/atw/left)"
    setprop sys.vr.log 1
    setprop sensor.debug.time 1
    sensor_log_date=$last_log_date
else
    title_str="\nUPTIME(s)\tCPU(F/L/T)\tGPU(F/L/T)\tVPU/HEVC(F)\tDDR(F/L)\tFPS"
fi

while true; do
    up_time=`uptime | busybox awk '{print $1}'`

    get_cpu_info
    prev_cpu_use=$cpu_use
    prev_cpu_total=$cpu_total

    if [ $skip_first -gt 0 ]; then
        skip_first=0
        sleep 1
        continue
    fi

    get_gpu_info
    get_ddr_info
    get_vpu_info
    get_disp_fps

    if [ $vr_mode == 1 ]; then
        get_vr_fps
        get_sensor_info
    fi

    if [ $(($loop_count%20)) -eq 0 ]; then
        echo $title_str
    fi

    if [ $vr_mode == 1 ]; then
        busybox printf "%s\t%03d/%02d/%02d\t%03d/%02d/%02d\t%03d/%03d\t\t%03d/%02d\t\t%03d/%03d/%03d\t%.1f/%.1f/%d\n" $up_time $cpu_freq $cpu_load $cpu_temp $gpu_freq $gpu_load $gpu_temp $vdpu_freq $hevc_freq $ddr_freq $ddr_load $sensor_min $sensor_avg $sensor_max $app_fps $atw_fps $left_count
    else
        busybox printf "%s\t%03d/%02d/%02d\t%03d/%02d/%02d\t%03d/%03d\t\t%03d/%02d\t\t%.1f\n" $up_time $cpu_freq $cpu_load $cpu_temp $gpu_freq $gpu_load $gpu_temp $vdpu_freq $hevc_freq $ddr_freq $ddr_load $app_fps
    fi

    cpu_load_total=$(($cpu_load_total+$cpu_load))
    cpu_freq_total=$(($cpu_freq_total+$cpu_freq))
    cpu_temp_total=$(($cpu_temp_total+$cpu_temp))

    gpu_load_total=$(($gpu_load_total+$gpu_load))
    gpu_freq_total=$(($gpu_freq_total+$gpu_freq))
    gpu_temp_total=$(($gpu_temp_total+$gpu_temp))

    ddr_load_total=$(($ddr_load_total+$ddr_load))
    ddr_freq_total=$(($ddr_freq_total+$ddr_freq))

    vpu_freq_total=$(($vpu_freq_total+$vdpu_freq))
    hevc_freq_total=$(($hevc_freq_total+$hevc_freq))

    app_fps_total=`echo "$app_fps_total $app_fps" | busybox awk '{print($1+$2)}'`
    if [ $vr_mode == 1 ]; then
        atw_fps_total=`echo "$atw_fps_total $atw_fps" | busybox awk '{print($1+$2)}'`
        left_total=$(($left_total+$left_count))
        sensor_min_total=$(($sensor_min_total+$sensor_min))
        sensor_avg_total=$(($sensor_avg_total+$sensor_avg))
        sensor_max_total=$(($sensor_max_total+$sensor_max))
    fi

    loop_count=$(($loop_count+1))

    if [ $loop_count -eq $max_loop_number ]; then
       break
    fi

    sleep $loop_delay
done

do_exit
