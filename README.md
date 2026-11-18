# hello_wayne - Aprendendo Kernel na pratica

Tres projetos de Kernel feitos do zero, rodando de verdade no Linux.

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

## Como compilar no WSL

    sudo ln -sfn ~/WSL2-Linux-Kernel /lib/modules/$(uname -r)/build

Feito por Wayne - de estudante a criador de Kernel.
