# Wayne Kernel Module - /proc/wayne

Modulo de kernel Linux interativo, compilado no Kernel 6.18.58.1 (WSL2).

Cria o arquivo /proc/wayne para leitura e escrita.

## Como usar
```bash
make
sudo insmod hello_wayne.ko
cat /proc/wayne
echo "Sou Kernel Developer" > /proc/wayne
cat /proc/wayne
sudo rmmod hello_wayne
dmesg | tail
```

## O que aprendi
- Compilar Kernel 6.18 do zero
- proc_create / proc_ops
- copy_to_user / copy_from_user
- Corrigir bug de snprintf com overlap de buffer
