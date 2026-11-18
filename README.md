# hello_wayne - Aprendendo Kernel na pratica

Dois projetos de Kernel feitos do zero, rodando de verdade no Linux.

## Projeto 1 - hello_wayne
Modulo que carrega no boot e conversa com o usuario.
- Escreve e le do Kernel
- CLI 2.0 com `count`, `reset`, `status`, `--help`, `--version`
- Contador de leituras

```bash
sudo insmod hello_wayne.ko
echo "OLHA EU RODANDO AO VIVO" > /proc/wayne
cat /proc/wayne
## Projeto 2 - WAYNE MONITOR 2.0
Seu proprio `top`, feito dentro do Kernel.

Mostra ao vivo:
- Uptime, Jiffies, CPU online
- Carga CPU (load 1 / 5 / 15 min)
- Processos totais e rodando agora
- Memoria total, livre e usada

```bash
cd projeto2-monitor
make
sudo insmod wayne_monitor.ko
watch -n 1 cat /proc/wayne_monitor
## Como compilar no WSL
Precisa do Kernel da Microsoft preparado:
```bash
sudo ln -sfn ~/WSL2-Linux-Kernel /lib/modules/$(uname -r)/build
```

Feito por Wayne - de estudante a criador de Kernel.
