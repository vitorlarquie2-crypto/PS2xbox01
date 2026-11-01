<!-- coderabbit-ai-generated:v1 -->

# Rastreamento da abertura em execução

## Resultado observado

O diagnóstico Linux com o binário da entrega anterior confirmou a entrada em `demo_title` com GP=277 e, em seguida, em `demo_op1` com GP=279. Nos dois pontos, o campo `0x01465e40` era zero. A pilha de `demo_op1` mostrou `isysWakeupSubThread` e `isysStartSubThread`, chamados a partir do código do MANAGER. Assim, nesta execução, a rotina de abertura foi iniciada pelo mecanismo de subthreads do jogo.

A Preview Chromium mostrou a apresentação Sony e depois a área cinza. As amostras do contador GP passaram por 0, 54, 261, 403 e 457; duas amostras posteriores conservaram GP=457 e o campo de título em zero. As pausas do debugger e o desempenho do host interferem no tempo observado. Isso não demonstra uma parada permanente nem identifica a causa.

O breakpoint em `opflowOp1End` permaneceu ativo e não registrou entrada até o encerramento deliberado desta execução. Os breakpoints de `demo_title` e `demo_op1` registraram uma entrada cada e foram desativados após o primeiro registro. Não se deve concluir que a rotina de liberação nunca possa ser chamada.

## Agendador e continuação

Uma amostra no estado cinza mostrou a thread EE 5 executando o serviço de relógio e outras threads em espera por sono ou semáforo. A thread principal do host estava em `EndDrawing`. Essa amostra não estabelece um impasse do agendador. A rotina `demo_op1` é iniciada por subthreads implementadas pelo próprio jogo; sua presença não pode ser inferida somente da lista de threads EE.

O código gerado de `demo_op1` chama `scrRun` em `0x013ab240` e contém uma consulta posterior em `0x013ab248`, antes da liberação em `0x013ab2a8`. `scrRun` encaminha ao executor em `0x013cf258`. A busca na RAM encontrou palavras com valor `0x013ab248` em `0x01c3da20` e `0x01c3da28`; esses valores são candidatos a endereços de retorno, sem validação suficiente para reconstruir uma pilha ou atribuir a espera ao executor de scripts.

O próximo diagnóstico deve observar a retomada dessas subthreads e a entrada/saída de `scrRun`, correlacionando-as com os eventos de quadro. Ainda não há evidência suficiente para alterar o agendador ou o fluxo do jogo. Nenhuma flag de título foi forçada.

## Reprodução e limites

- Fonte do runtime inalterada desde `e90bc608872921d06f1a0d4d38962b3115160703`; documentação inicial deste turno em `1aca823`.
- Binário externo `ps2EntryRunner`: SHA256 `6a1bccdcede2764306ccf049e2257504164a7a921bcc425b81e906ef26fcc2f2`. Não foi recompilado neste diagnóstico.
- ELF SCUS_974.72: SHA256 `d16ad76ba8cdf87695d94b1316d51c1f248959ec0749814c31bb7a2d55d9a2e1`.
- Ubuntu 26.04.1, Xvfb :99, OpenGL por software, FFmpeg desativado e áudio não validado. Entrada por xdotool, visualização por noVNC/Chromium compartilhado.
- GDB com breakpoints em `demo_op1_0x13aadf0`, `opflowOp1End_0x1357648` e `demo_title_0x1402fe0`. Os comandos de impressão foram executados dentro dos breakpoints para distinguir entradas reais de interrupções manuais.
- Um objeto auxiliar contendo somente tipos dos cabeçalhos atuais foi compilado e carregado como símbolos do debugger. Não foi vinculado ao runner nem executado. A comparação do ponteiro de RAM do agendador com o ponteiro capturado no jogo foi positiva. As leituras de estruturas dependem da compatibilidade desse layout; os breakpoints e leituras diretas de RAM independem dele.
- Primeiro diagnóstico em modo batch encerrado por SIGTERM; segundo em modo interativo encerrado por `kill` e `quit` no GDB. Nenhum processo do jogo foi deixado em execução.

Logs e RAM privados permanecem em `inputs/sotc/runs/`. O log interativo tem SHA256 `22ea578722f85b103c1c94b4fde811c45d4b2ac6f1e5195896bc6df1be7e3643`. Os dados do disco e o código gerado não são incorporados ao Git. Este resultado não comprova jogabilidade nem integração do runtime ao Xbox; o diagnóstico UWP independente continua sendo o pacote cujo funcionamento o usuário confirmou no console.
