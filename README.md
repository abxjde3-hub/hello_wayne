# hello_wayne - Meu primeiro driver de Kernel

Driver que cria 3 arquivos no /proc e um comando CLI!

## Arquivos
- /proc/wayne - mensagem (le e escreve)
- /proc/wayne_count - contador de leituras
- /proc/wayne_reset - zera o contador

## Comando
```bash
sudo insmod hello_wayne.ko
wayne
wayne "Wayne e brabo"
wayne count
wayne reset
