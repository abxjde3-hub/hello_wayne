# APK + PERSISTENCIA - Ambiente Real - Projeto 51
## Como funciona de verdade - Nao e nome fixo!

### 1. Nomes NAO sao fixos:
- Nao deixar nome fixo no codigo
- Nao trocar Device 001 por nome fixo
- O nome vem do Android quando conectar

### 2. Como aparece:
- Celular Android instala o APK do WAYNE TOOL
- APK conecta no servidor
- Software le o modelo real do Android
- Software imprime na tela: modelo, pais, bateria, ping
- Exemplo: quando conectar, aparece o modelo real dele

### 3. Persistir no local - Sem deslogar quando cair:
- Quando conectar, salvar no navegador: localStorage
- Salvar: nome, modelo, bateria, ping, posicao na grade
- Quando o sistema cair, quando fechar e abrir:
    - Nao desloga!
    - Nao some!
    - Carrega do localStorage e mostra de novo, no mesmo lugar!
- So sai quando o cliente clicar em Desconectar

### 4. Compativel com todos:
- Todos os Android compativeis
- Qualquer modelo - se conectar via APK, aparece
- Se nao conectar, nao aparece - fica vazio, esperando

### 5. Teste no bash - Sem navegador:
- Verificar se o arquivo tem localStorage
- Verificar se tem funcao de salvar e carregar

### Versao:
- v6.1 entregue - pronto cliente
- Proximo: v6.2 - APK + Persistencia
