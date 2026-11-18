# hello_wayne - Aprendendo Kernel na pratica

Seis projetos de Kernel feitos do zero, rodando de verdade no Linux.

## Projeto 1 - hello_wayne

Modulo que carrega no boot e conversa com o usuario.

- Escreve e le do Kernel
- CLI 2.0 com count, reset, status, --help, --version
- Contador de leituras

Exemplo:

    sudo insmod hello_wayne.ko
    echo "OLHA EU RODANDO AO VIVO" > /proc/wayne
    cat /proc/wayne

## Projeto 2 - WAYNE MONITOR 2.0

Seu proprio top, feito dentro do Kernel.

Mostra ao vivo:
- Uptime, Jiffies, CPU online
- Carga CPU (load 1 / 5 / 15 min)
- Processos totais e rodando agora
- Memoria total, livre e usada

Exemplo:

    cd projeto2-monitor
    make
    sudo insmod wayne_monitor.ko
    watch -n 1 cat /proc/wayne_monitor

## Projeto 3 - WAYNE TIMER 3.0

Relogio dentro do Kernel, disparando sozinho a cada 1 segundo.

Mostra ao vivo:
- Segundos rodando
- Vezes que o timer disparou
- Jiffies atual e HZ do sistema

Aprendizado: timer_setup, mod_timer, timer_delete_sync (Kernel 6.18), jiffies

Exemplo:

    cd projeto3-timer
    make
    sudo insmod wayne_timer.ko
    watch -n 1 cat /proc/wayne_timer

## Projeto 4 - WAYNE CHAR DEVICE 4.0

Driver de verdade com dispositivo em /dev/wayne.

- open, release, read, write
- ioctl CLEAR e GET_COUNT
- major dinamico, class_create, device_create

Exemplo:

    cd projeto4-chardev
    make
    sudo insmod wayne_chardev.ko
    sudo chmod 666 /dev/wayne
    echo "OLA KERNEL" | sudo tee /dev/wayne
    cat /dev/wayne

## Projeto 5 - WAYNE SYSFS 5.0

Controle do Kernel pelo /sys, igual driver de verdade faz.

Arquivos criados:
- /sys/kernel/wayne/modo (le e escreve)
- /sys/kernel/wayne/mensagem (le e escreve)
- /sys/kernel/wayne/contador (so le)
- /sys/kernel/wayne/versao (so le)

Aprendizado: kobject_create_and_add, sysfs_create_group, __ATTR

Exemplo:

    cd projeto5-sysfs
    make
    sudo insmod wayne_sysfs.ko
    ls -l /sys/kernel/wayne/
    echo "modo turbo" | sudo tee /sys/kernel/wayne/modo
    cat /sys/kernel/wayne/contador

## Projeto 6 - WAYNE NET 6.0

Gancho no Kernel que ve todos os pacotes de rede, igual firewall faz.

Conta ao vivo:
- Total de pacotes
- TCP, UDP, ICMP (ping), Outros
- Ultimo IP origem

Aprendizado: netfilter, NF_INET_PRE_ROUTING, ip_hdr, nf_register_net_hook

Exemplo:

    cd projeto6-net
    make
    sudo insmod wayne_net.ko
    ping -c 3 8.8.8.8
    cat /proc/wayne_net

## Como compilar no WSL

    sudo ln -sfn ~/WSL2-Linux-Kernel /lib/modules/$(uname -r)/build

Feito por Wayne - de estudante a criador de Kernel.
