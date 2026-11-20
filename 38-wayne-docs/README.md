# Projeto 38 - WAYNE DOCS - Documentacao

hello_wayne - Wayne Lab - Wayne Docs Paw

37 projetos - v4.0 - codigo limpo

## Como usar

```bash
cd 38-wayne-docs && make
sudo insmod wayne_docs.ko
cat /sys/kernel/wayne/docs
cat /sys/kernel/wayne/press
echo 1 | sudo tee /sys/kernel/wayne/press
sudo rmmod wayne_docs
```

## O que ele faz

- Cria Wayne Docs Paw como input
- Mostra documentacao em /sys/kernel/wayne/docs
- Aperta KEY_D a cada press
